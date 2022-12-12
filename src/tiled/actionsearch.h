/*
 * actionsearch.h
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

#include "locatorwidget.h"

namespace Tiled {

class ActionMatchesModel;

class ActionLocatorSource : public LocatorSource
{
public:
    explicit ActionLocatorSource();
    ~ActionLocatorSource();

    QString placeholderText() const override;
    QAbstractListModel *model() const override;
    void setFilterWords(const QStringList &words) override;
    void activate(const QModelIndex &index) override;

private:
    std::unique_ptr<ActionMatchesModel> mModel;
};

} // namespace Tiled
