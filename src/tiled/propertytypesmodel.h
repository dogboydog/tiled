/*
 * propertytypesmodel.h
 * Copyright 2011, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
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

#include "object.h"
#include "properties.h"
#include "propertytype.h"

#include <QAbstractListModel>
#include <QIcon>
#include <QStringList>

namespace Tiled {

class PropertyTypesModel : public QAbstractListModel
{
    Q_OBJECT

public:
    PropertyTypesModel(QObject *parent = nullptr);

    void setPropertyTypes(const SharedPropertyTypes &propertyTypes);

    PropertyType *propertyTypeAt(const QModelIndex &index) const;

    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    void setPropertyTypeName(int row, const QString &name);
    void removePropertyTypes(const QModelIndexList &indexes);

    QModelIndex addNewPropertyType(PropertyType::Type type);

signals:
    void nameChanged(const QModelIndex &index, const PropertyType &type);

private:
    QString nextPropertyTypeName(PropertyType::Type type) const;

    SharedPropertyTypes mPropertyTypes;

    QIcon mEnumIcon;
    QIcon mClassIcon;
};

} // namespace Tiled
