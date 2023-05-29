/*
 * project.h
 * Copyright 2019, Thorbjørn Lindeijer <bjorn@lindeijer.nl>
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

#include "command.h"
#include "object.h"
#include "properties.h"
#include "propertytype.h"
#include "tiled.h"

#include <QDateTime>
#include <QStringList>
#include <QVector>

namespace Tiled {

class Project : public Object
{
public:
    Project();

    const QString &fileName() const;
    bool save();
    bool save(const QString &fileName);
    bool load(const QString &fileName);

    void addFolder(const QString &folder);
    void removeFolder(int index);
    const QStringList &folders() const;

    const QDateTime &lastSaved() const;
    const SharedPropertyTypes &propertyTypes() const;

    QString mExtensionsPath;
    QString mObjectTypesFile;
    QString mAutomappingRulesFile;
    QVector<Command> mCommands;
    CompatibilityVersion mCompatibilityVersion = Tiled_Current;
    Project &operator =(const Project &value);

private:
    QDateTime mLastSaved;
    QString mFileName;
    QStringList mFolders;
    SharedPropertyTypes mPropertyTypes;
};


inline const QString &Project::fileName() const
{
    return mFileName;
}

inline const QStringList &Project::folders() const
{
    return mFolders;
}

inline const QDateTime &Project::lastSaved() const
{
    return mLastSaved;
}

inline const SharedPropertyTypes &Project::propertyTypes() const
{
    return mPropertyTypes;
}
inline Project &Project::operator =(const Project &value)
{ 
    if (this == &value) {
            return *this;
    }
    mExtensionsPath = value.mExtensionsPath;
    mObjectTypesFile = value.mObjectTypesFile;
    mAutomappingRulesFile = value.mAutomappingRulesFile;
    mCompatibilityVersion = value.mCompatibilityVersion;
    mCommands = value.mCommands;
    mLastSaved = value.mLastSaved;
    mFileName = value.mFileName;
    mFolders = value.mFolders;
    mPropertyTypes = value.mPropertyTypes;
    setClassName(value.className());
    setProperties(value.properties());
    return *this; 
}
} // namespace Tiled
