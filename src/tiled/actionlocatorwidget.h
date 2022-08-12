/*
 *  actionlocatorwidget.h
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
#pragma once

#include <QFrame>
#include <QAction>
#include "id.h"
#include "locatorwidget.h"

namespace Tiled {

    class FilterEdit;
    class ActionMatchesModel;

    class ActionLocatorWidget : public LocatorWidget
    {
        Q_OBJECT


    public:
        struct Match {
            int score;
            Id actionId;
            bool isFromScript;
            QString text;
        };

        explicit ActionLocatorWidget(QWidget *parent = nullptr);

        void setVisible(bool visible) override;

    private:
        void setFilterText(const QString &text);

        FilterEdit *mFilterEdit;
        ResultsView *mActionResultsView;
        ActionMatchesModel *mListModel;
        MatchDelegate *mDelegate;
        const QFont small;
        const QFont big;
    };

} // namespace Tiled
