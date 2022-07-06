/*
 *  actionsearchwidget.cpp
 * Copyright 2022, Chris Boehm AKA dogboydog
 * Copyright 2022, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
 *
 * This file is part of Tiled.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "actionsearchwidget.h"
#include "documentmanager.h"
#include "filteredit.h"
#include "actionmanager.h"
#include "projectmanager.h"
#include "projectmodel.h"
#include "rangeset.h"
#include "scriptmanager.h"
#include "scriptmodule.h"
#include "utils.h"

#include <QAbstractListModel>
#include <QApplication>
#include <QDir>
#include <QKeyEvent>
#include <QPainter>
#include <QRegularExpression>
#include <QScrollBar>
#include <QStaticText>
#include <QStyledItemDelegate>
#include <QTreeView>
#include <QVBoxLayout>

#include <QDebug>
#include <QLabel>

namespace Tiled {

    class ActionMatchesModel : public QAbstractListModel
    {
    public:
        explicit ActionMatchesModel(QObject *parent = nullptr);

        int rowCount(const QModelIndex &parent) const override;
        QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

        const QVector<ActionSearchWidget::Match> &matches() const { return mMatches; }
        void setMatches(QVector<ActionSearchWidget::Match> matches);

    private:
        QVector<ActionSearchWidget::Match> mMatches;
    };

    ActionMatchesModel::ActionMatchesModel(QObject *parent)
            : QAbstractListModel(parent)
    {}

    int ActionMatchesModel::rowCount(const QModelIndex &parent) const
    {
        return parent.isValid() ? 0 : mMatches.size();
    }

    QVariant ActionMatchesModel::data(const QModelIndex &index, int role) const
    {
        switch (role) {
            case Qt::DisplayRole: {
                const ActionSearchWidget::Match &match = mMatches.at(index.row());
                return match.text;
            }
        }
        return QVariant();
    }

    void ActionMatchesModel::setMatches(QVector<ActionSearchWidget::Match> matches)
    {
        beginResetModel();
        mMatches = std::move(matches);
        endResetModel();
    }

///////////////////////////////////////////////////////////////////////////////

    static QFont scaledFont(const QFont &font, qreal scale)
    {
        QFont scaled(font);
        if (font.pixelSize() > 0)
            scaled.setPixelSize(font.pixelSize() * scale);
        else
            scaled.setPointSizeF(font.pointSizeF() * scale);
        return scaled;
    }

    class ActionMatchDelegate : public QStyledItemDelegate
    {
    public:
        ActionMatchDelegate(QObject *parent = nullptr);

        QSize sizeHint(const QStyleOptionViewItem &option,
                       const QModelIndex &index) const override;

        void paint(QPainter *painter,
                   const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;

        void setWords(const QStringList &words) { mWords = words; }

    private:
        class Fonts {
        public:
            Fonts(const QFont &base)
                    : small(scaledFont(base, 0.9))
                    , big(scaledFont(base, 1.2))
            {}

            const QFont small;
            const QFont big;
        };

        QStringList mWords;
    };

    ActionMatchDelegate::ActionMatchDelegate(QObject *parent)
            : QStyledItemDelegate(parent)
    {}

    QSize ActionMatchDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &) const
    {
        const QFont bigFont = scaledFont(option.font, 1.2);
        const QFontMetrics bigFontMetrics(bigFont);

        const int margin = Utils::dpiScaled(2);
        return QSize(margin * 2, margin * 2 + bigFontMetrics.lineSpacing() * 2);
    }

    void ActionMatchDelegate::paint(QPainter *painter,
                              const QStyleOptionViewItem &option,
                              const QModelIndex &index) const
    {
        painter->save();

        QString filePath = index.data().toString();
        const int lastSlash = filePath.lastIndexOf(QLatin1Char('/'));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
        const auto ranges = Utils::matchingRanges(mWords, &filePath);
#else
        const auto ranges = Utils::matchingRanges(mWords, filePath);
#endif

        filePath = QDir::toNativeSeparators(filePath);

        // Since we're using HTML to markup the entries we'll need to escape the
        // filePath and fileName to avoid them introducing any formatting, however
        // unlikely this may be.
        QString filePathHtml;
        QString fileNameHtml;
        int filePathIndex = 0;

        auto escapedRange = [&] (int first, int last) -> QString {
            return filePath.mid(first, last - first + 1).toHtmlEscaped();
        };

        for (const auto &range : ranges) {
            if (range.first > filePathIndex)
                filePathHtml.append(escapedRange(filePathIndex, range.first - 1));

            filePathHtml.append(QStringLiteral("<b>"));
            filePathHtml.append(escapedRange(range.first, range.second));
            filePathHtml.append(QStringLiteral("</b>"));

            if (range.second > lastSlash) {
                const auto first = qMax(range.first, lastSlash + 1);
                const auto fileNameIndex = qMax(filePathIndex, lastSlash + 1);

                if (first > fileNameIndex)
                    fileNameHtml.append(escapedRange(fileNameIndex, first - 1));

                fileNameHtml.append(QStringLiteral("<b>"));
                fileNameHtml.append(escapedRange(first, range.second));
                fileNameHtml.append(QStringLiteral("</b>"));
            }

            filePathIndex = range.second + 1;
        }

        filePathHtml.append(escapedRange(filePathIndex, filePath.size() - 1));
        fileNameHtml.append(escapedRange(qMax(filePathIndex, lastSlash + 1), filePath.size() - 1));

        const Fonts fonts(option.font);
        const QFontMetrics bigFontMetrics(fonts.big);

        const int margin = Utils::dpiScaled(2);
        const auto fileNameRect = option.rect.adjusted(margin, margin, -margin, 0);
        const auto filePathRect = option.rect.adjusted(margin, margin + bigFontMetrics.lineSpacing(), -margin, 0);

        // draw the background (covers selection)
        QStyle *style = QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

        // adjust text color to state
        QPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? QPalette::Normal : QPalette::Disabled;
        if (cg == QPalette::Normal && !(option.state & QStyle::State_Active))
            cg = QPalette::Inactive;
        if (option.state & QStyle::State_Selected) {
            painter->setPen(option.palette.color(cg, QPalette::HighlightedText));
        } else {
            painter->setPen(option.palette.color(cg, QPalette::Text));
        }

        QTextOption textOption;
        textOption.setWrapMode(QTextOption::NoWrap);

        QStaticText staticText(fileNameHtml);
        staticText.setTextOption(textOption);
        staticText.setTextFormat(Qt::RichText);
        staticText.setTextWidth(fileNameRect.width());
        staticText.prepare(painter->transform(), fonts.big);

        painter->setFont(fonts.big);
        painter->drawStaticText(fileNameRect.topLeft(), staticText);

        staticText.setText(filePathHtml);
        staticText.prepare(painter->transform(), fonts.small);

        painter->setOpacity(0.75);
        painter->setFont(fonts.small);
        painter->drawStaticText(filePathRect.topLeft(), staticText);

        // draw the focus rect
        if (option.state & QStyle::State_HasFocus) {
            QStyleOptionFocusRect o;
            o.QStyleOption::operator=(option);
            o.rect = style->subElementRect(QStyle::SE_ItemViewItemFocusRect, &option);
            o.state |= QStyle::State_KeyboardFocusChange;
            o.state |= QStyle::State_Item;
            QPalette::ColorGroup cg = (option.state & QStyle::State_Enabled)
                                      ? QPalette::Normal : QPalette::Disabled;
            o.backgroundColor = option.palette.color(cg, (option.state & QStyle::State_Selected)
                                                         ? QPalette::Highlight : QPalette::Window);
            style->drawPrimitive(QStyle::PE_FrameFocusRect, &o, painter);
        }

        painter->restore();
    }

///////////////////////////////////////////////////////////////////////////////

    class ActionResultsView : public QTreeView
    {
    public:
        explicit ActionResultsView(QWidget *parent = nullptr);

        QSize sizeHint() const override;

        void updateMaximumHeight();

    protected:
        void keyPressEvent(QKeyEvent *event) override;
    };

    ActionResultsView::ActionResultsView(QWidget *parent)
            : QTreeView(parent)
    {
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    }

    QSize ActionResultsView::sizeHint() const
    {
        return QSize(Utils::dpiScaled(600), Utils::dpiScaled(600));
    }

    void ActionResultsView::updateMaximumHeight()
    {
        int maximumHeight = 0;

        if (auto m = model()) {
            int rowCount = m->rowCount();
            if (rowCount > 0) {
                const int itemHeight = indexRowSizeHint(m->index(0, 0));
                maximumHeight = itemHeight * rowCount;
            }
        }

        setMaximumHeight(maximumHeight);
    }

    inline void ActionResultsView::keyPressEvent(QKeyEvent *event)
    {
        // Make sure the Enter and Return keys activate the current index. This
        // doesn't happen otherwise on macOS.
        switch (event->key()) {
            case Qt::Key_Enter:
            case Qt::Key_Return:
                if (currentIndex().isValid())
                    emit activated(currentIndex());
                return;
        }

        QTreeView::keyPressEvent(event);
    }

///////////////////////////////////////////////////////////////////////////////
    static QTextStream& stdOut() // TODO REMOVE
    {
        static QTextStream ts(stdout);
        return ts;
    }

    ActionSearchWidget::ActionSearchWidget(QWidget *parent)
            : QFrame(parent, Qt::Popup)
            , mFilterEdit(new FilterEdit(this))
            , mActionResultsView(new ActionResultsView(this))
            , mListModel(new ActionMatchesModel(this))
            , mDelegate(new ActionMatchDelegate(this))
    {
        setAttribute(Qt::WA_DeleteOnClose);
        setAttribute(Qt::WA_InputMethodEnabled);
        setFrameStyle(QFrame::StyledPanel | QFrame::Plain);

        mActionResultsView->setUniformRowHeights(true);
        mActionResultsView->setRootIsDecorated(false);
        mActionResultsView->setItemDelegate(mDelegate);
        mActionResultsView->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        mActionResultsView->setModel(mListModel);
        mActionResultsView->setHeaderHidden(true);

        mFilterEdit->setPlaceholderText(tr("Search Tiled Actions..."));
        mFilterEdit->setFilteredView(mActionResultsView);
        mFilterEdit->setClearTextOnEscape(false);
        mFilterEdit->setFont(scaledFont(mFilterEdit->font(), 1.5));

        setFocusProxy(mFilterEdit);
        mActionResultsView->setFocusProxy(mFilterEdit);

        mActionResultsView->setFrameShape(QFrame::NoFrame);
        mActionResultsView->viewport()->setBackgroundRole(QPalette::Window);

        auto margin = Utils::dpiScaled(4);
        auto verticalLayout = new QVBoxLayout;
        verticalLayout->setContentsMargins(margin, margin, margin, margin);
        verticalLayout->setSpacing(margin);
        verticalLayout->addWidget(mFilterEdit);
        verticalLayout->addWidget(mActionResultsView);
        verticalLayout->addStretch(0);
        setLayout(verticalLayout);

        connect(mFilterEdit, &QLineEdit::textChanged, this, &ActionSearchWidget::setFilterText);
        connect(mActionResultsView, &QAbstractItemView::activated, this, [this] (const QModelIndex &index) {
            const Id actionId = mListModel->matches().at(index.row()).actionId;
            close();
            ActionManager::instance()->action(actionId)->trigger();
        });
    }

    void ActionSearchWidget::setVisible(bool visible)
    {
        QFrame::setVisible(visible);

        if (visible) {
            setFocus();

            if (!mFilterEdit->text().isEmpty())
                mFilterEdit->clear();
            else
                setFilterText(QString());
        }
    }
    static QVector<ActionSearchWidget::Match> findActions(const QStringList &words)
    {
        QList<Id> actions = ActionManager::actions();
        QList<Id> menus = ActionManager::menus();
        for (const Id &actionId: actions){
            stdOut() << "Found action ID " << actionId.name() << Qt::endl;
        }
        QVector<ActionSearchWidget::Match> result;
        for (const auto &tiledAction: actions) {
            QAction *tiledQAction = ActionManager::findAction(tiledAction.name());

            QString actionText = tiledQAction->text();
            QStringRef actionTextRef(&actionText);
            const int totalScore = Utils::matchingScore(words, actionTextRef);
            // remove single & characters
            QRegularExpression re(QString::fromUtf8("(?<=^|[^&])&"));
            // avoid changing the original text
            QString sanitizedText = tiledQAction->text();
            sanitizedText.replace(re, QString());
            if (totalScore > 0) {
                result.append(ActionSearchWidget::Match {
                        totalScore,
                        .actionId = tiledAction,
                        .isFromScript = false,
                        .text =sanitizedText
                });
            }
        }
        return result;
    }
    void ActionSearchWidget::setFilterText(const QString &text)
    {
        const QString normalized = QDir::fromNativeSeparators(text);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
        const QStringList words = normalized.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
        const QStringList words = normalized.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif

        // TODO: test code begin

        auto matches = findActions(words);
        std::stable_sort(matches.begin(), matches.end(), [] (const ActionSearchWidget::Match &a, const ActionSearchWidget::Match &b) {
            // Sort based on score first
            if (a.score != b.score)
                return a.score > b.score;

            // If score is the same, sort alphabetically
            return a.text.compare(b.text, Qt::CaseInsensitive) < 0;
        });

        for (const ActionSearchWidget::Match  match: matches){
            stdOut() << "Action matched: " << match.text << (match.isFromScript? ", from script": ", not from script") << Qt::endl;
        }
        mDelegate->setWords(words);
        mListModel->setMatches(matches);

        mActionResultsView->updateGeometry();
        mActionResultsView->updateMaximumHeight();

        // Restore or introduce selection
        if (!matches.isEmpty())
            mActionResultsView->setCurrentIndex(mListModel->index(0));

        // TODO test code end
        layout()->activate();
        resize(sizeHint());
    }

} // namespace Tiled

#include "moc_actionsearchwidget.cpp"
