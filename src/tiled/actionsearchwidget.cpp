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
            , mDelegate(new MatchDelegate(this))
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
