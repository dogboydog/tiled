/*
 * propertytypeseditor.h
 * Copyright 2016-2021, Thorbjørn Lindeijer <bjorn@lindeijer.nl>>
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

#include "properties.h"

#include <QDialog>
#include <QIcon>

class QLabel;
class QStackedLayout;
class QStringListModel;
class QTreeView;

class QtBrowserItem;
class QtTreePropertyBrowser;

namespace Ui {
class PropertyTypesEditor;
}

namespace Tiled {

class CustomPropertiesHelper;
class PropertyTypesModel;

class PropertyTypesEditor : public QDialog
{
    Q_OBJECT

public:
    explicit PropertyTypesEditor(QWidget *parent = nullptr);
    ~PropertyTypesEditor() override;

signals:
    void closed();

protected:
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *e) override;

private:
    void addPropertyType(PropertyType::Type type);
    void selectedPropertyTypesChanged();
    void removeSelectedPropertyTypes();
    QModelIndex selectedPropertyTypeIndex() const;
    PropertyType *selectedPropertyType() const;

    void currentMemberItemChanged(QtBrowserItem *item);

    void propertyTypeNameChanged(const QModelIndex &index,
                                 const PropertyType &type);
    void applyMemberToSelectedType(const QString &name, const QVariant &value);
    void applyPropertyTypes();
    void propertyTypesChanged();

    void updateDetails();
    void updateActions();

    void addValue();
    void removeValues();

    void openAddMemberDialog();
    void addMember(const QString &name, const QVariant &value = QVariant());
    void editMember(const QString &name);
    void removeMember();
    void renameMember();
    void renameMemberTo(const QString &name);

    void selectFirstPropertyType();
    void valuesChanged();
    void nameChanged(const QString &name);

    void memberValueChanged(const QString &name, const QVariant &value);

    void retranslateUi();

    Ui::PropertyTypesEditor *mUi;
    PropertyTypesModel *mPropertyTypesModel;
    QTreeView *mValuesView;
    QWidget *mValuesWithToolBarWidget;
    QStringListModel *mValuesModel;
    QtTreePropertyBrowser *mMembersView;
    QWidget *mMembersWithToolBarWidget;
    CustomPropertiesHelper *mPropertiesHelper;
    QStackedLayout *mValuesAndMembersStack;
    QLabel *mValuesOrMembersLabel;

    bool mSettingPrefPropertyTypes = false;
    bool mSettingName = false;
    bool mUpdatingDetails = false;

    QAction *mAddEnumPropertyTypeAction;
    QAction *mAddClassPropertyTypeAction;
    QAction *mRemovePropertyTypeAction;

    QAction *mAddValueAction;
    QAction *mRemoveValueAction;

    QAction *mAddMemberAction;
    QAction *mRemoveMemberAction;
    QAction *mRenameMemberAction;

    QIcon mEnumIcon;
    QIcon mClassIcon;
    QAction *mNameEditIconAction;
};

} // namespace Tiled
