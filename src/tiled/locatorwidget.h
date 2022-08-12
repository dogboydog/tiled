/*
 * locatorwidget.h
 * Copyright 2020, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#pragma once

#include <QFrame>
#include <QStyledItemDelegate>
#include <QTreeView>
namespace Tiled {

class FilterEdit;
class MatchDelegate : public QStyledItemDelegate
{
public:
    MatchDelegate(QFont smallFont, QFont bigFont, QObject *parent = nullptr);

    QSize sizeHint(const QStyleOptionViewItem &option,
                  const QModelIndex &index) const override;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;

    void setWords(const QStringList &words) { mWords = words; }
    QFont big;
    QFont small;
protected:
   QStringList mWords;

};

class ResultsView : public QTreeView
{
public:
    explicit ResultsView(QWidget *parent = nullptr);

    QSize sizeHint() const override;

    void updateMaximumHeight();

protected:
    void keyPressEvent(QKeyEvent *event) override;
};
class MatchesModel;
class ResultsView;

class LocatorWidget : public QFrame
{
    Q_OBJECT

public:
    explicit LocatorWidget(MatchDelegate *matchDelegate, FilterEdit *filterEdit,
                           QFont smallFont, QFont bigFont,
                           QWidget *parent = nullptr);
    void setVisible(bool visible) override;

protected:
    void setFilterText(const QString &text);
    FilterEdit *mFilterEdit;
    MatchDelegate *mDelegate;
    QFont small;
    QFont big;
};

class ProjectFileLocatorWidget: public LocatorWidget {

public:
    explicit ProjectFileLocatorWidget(QWidget *parent, const QFont base);
private:
    MatchesModel *mListModel;

    ResultsView *mResultsView;

};
} // namespace Tiled
