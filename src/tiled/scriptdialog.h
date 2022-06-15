/*
 * scriptdialog.h
 * Copyright 2020, David Konsumer <konsumer@jetboystudio.com>
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

#include "mainwindow.h"
#include <QString>
#include <QObject>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QLabel>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QPushButton>
#include <QComboBox>
#include <QCoreApplication>
#include <QDialog>
#include "colorbutton.h"
#include <QString>
#include <QLineEdit>
#include <QList>
#include <QSize>
#include <QResizeEvent>
class QJSEngine;

namespace Tiled {
class ScriptDialog : public QDialog
{
    Q_OBJECT

public:
    Q_INVOKABLE ScriptDialog();
    Q_INVOKABLE ScriptDialog(const QString &title, const int width, const int height);
    ~ScriptDialog() override;

    bool checkForClosed() const;

    Q_INVOKABLE void setTitle(const QString &title);
    Q_INVOKABLE QLabel *addLabel(const QString &text, bool maxWidth = false);
    Q_INVOKABLE QFrame *addSeparator(const QString &labelText = QString());
    Q_INVOKABLE QLineEdit *addTextInput(const QString &labelText= QString(), const QString &defaultValue= QString());
    Q_INVOKABLE QDoubleSpinBox *addNumberInput(const QString &labelText);
    Q_INVOKABLE QSlider *addSlider(const QString &labelText);
    Q_INVOKABLE QComboBox *addComboBox(const QString &labelText, const QStringList &values);
    Q_INVOKABLE QCheckBox *addCheckBox(const QString &labelText, bool defaultValue);
    Q_INVOKABLE QPushButton *addButton(const QString &labelText);
    Q_INVOKABLE Tiled::ColorButton *addColorButton(const QString &labelText);
    Q_INVOKABLE void resize(const int width, const int height);
    Q_INVOKABLE void clear();
    Q_INVOKABLE void addNewRow();
private:
    int m_rowIndex = 0;
    int m_widgetsInRow = 0;
    int m_leftColumnStretch = 1;
    int m_rightColumnStretch = 4;
    QGridLayout *m_gridLayout;
    QLabel *newLabel(const QString& labelText);
    void initializeLayout();
    QWidget *m_rowLayoutWidget;
    QHBoxLayout* m_rowLayout;
    QString m_lastWidgetTypeName;
    bool checkIfSameType(const char *newTypeName);
    void addDialogWidget(QWidget * widget);
    void moveToColumn2();
    void deleteWidgetsFromLayout(QLayout * layout);
};

void registerDialog(QJSEngine *jsEngine);

} // namespace Tiled
Q_DECLARE_METATYPE(Tiled::ScriptDialog*);
Q_DECLARE_METATYPE(QCheckBox*)
Q_DECLARE_METATYPE(QPushButton*)
Q_DECLARE_METATYPE(QSlider*)
Q_DECLARE_METATYPE(QLabel*)
Q_DECLARE_METATYPE(QLineEdit*)
