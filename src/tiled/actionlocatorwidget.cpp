/*
 *  actionlocatorwidget.cpp
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

#include "actionlocatorwidget.h"
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

        const QVector<ActionLocatorWidget::Match> &matches() const { return mMatches; }
        void setMatches(QVector<ActionLocatorWidget::Match> matches);

    private:
        QVector<ActionLocatorWidget::Match> mMatches;
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
                const ActionLocatorWidget::Match &match = mMatches.at(index.row());
                return match.text;
            }
        }
        return QVariant();
    }

    void ActionMatchesModel::setMatches(QVector<ActionLocatorWidget::Match> matches)
    {
        beginResetModel();
        mMatches = std::move(matches);
        endResetModel();
    }

///////////////////////////////////////////////////////////////////////////////

ActionLocatorWidget::ActionLocatorWidget(QWidget *parent, const QFont &base)
        : LocatorWidget(new MatchDelegate(scaledFont(base, 0.9), scaledFont(base, 1.2), this), new FilterEdit(this),
                        scaledFont(base, 0.9), scaledFont(base, 1.2)), parent)
        , mActionResultsView(new ResultsView(this))
        , mListModel(new ActionMatchesModel(this))
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

    connect(mFilterEdit, &QLineEdit::textChanged, this, &ActionLocatorWidget::setFilterText);
    connect(mActionResultsView, &QAbstractItemView::activated, this, [this] (const QModelIndex &index) {
        const Id actionId = mListModel->matches().at(index.row()).actionId;
        close();
        ActionManager::instance()->action(actionId)->trigger();
    });
}

void ActionLocatorWidget::setVisible(bool visible)
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
static QVector<ActionLocatorWidget::Match> findActions(const QStringList &words)
{
    QList<Id> actions = ActionManager::actions();
    // TODO: determine which menu an action is in if any?
    QList<Id> menus = ActionManager::menus();

    QVector<ActionLocatorWidget::Match> result;
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
            result.append(ActionLocatorWidget::Match {
                    totalScore,
                    .actionId = tiledAction,
                    .isFromScript = false,
                    .text =sanitizedText
            });
        }
    }
    return result;
}
void ActionLocatorWidget::setFilterText(const QString &text)
{
    const QString normalized = QDir::fromNativeSeparators(text);
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    const QStringList words = normalized.split(QLatin1Char(' '), QString::SkipEmptyParts);
#else
    const QStringList words = normalized.split(QLatin1Char(' '), Qt::SkipEmptyParts);
#endif

    auto matches = findActions(words);
    std::stable_sort(matches.begin(), matches.end(), [] (const ActionLocatorWidget::Match &a, const ActionLocatorWidget::Match &b) {
        // Sort based on score first
        if (a.score != b.score)
            return a.score > b.score;

        // If score is the same, sort alphabetically
        return a.text.compare(b.text, Qt::CaseInsensitive) < 0;
    });

    mDelegate->setWords(words);
    mListModel->setMatches(matches);

    mActionResultsView->updateGeometry();
    mActionResultsView->updateMaximumHeight();

    // Restore or introduce selection
    if (!matches.isEmpty())
        mActionResultsView->setCurrentIndex(mListModel->index(0));

    layout()->activate();
    resize(sizeHint());
}

} // namespace Tiled

#include "moc_actionlocatorwidget.cpp"
