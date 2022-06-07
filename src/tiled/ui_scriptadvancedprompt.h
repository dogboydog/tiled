/*
 * ScriptAdvancedPrompt.h
 * Copyright 2008-2009, Thorbjørn Lindeijer <thorbjorn@lindeijer.nl>
 * Copyright 2009, Dennis Honeyman <arcticuno@gmail.com>
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

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QTextBrowser>
#include <QtWidgets/QVBoxLayout>
#include "newversionbutton.h"

QT_BEGIN_NAMESPACE
static const char ok[] = "OK";
class Ui_ScriptAdvancedPrompt
{
public:
     QPushButton *okButton;
     QHBoxLayout *buttonLayout;
    void setupUi(QDialog *Ui_ScriptAdvancedPrompt)
    {
        if (Ui_ScriptAdvancedPrompt->objectName().isEmpty())
            Ui_ScriptAdvancedPrompt->setObjectName(QString::fromUtf8("AboutDialog"));
        Ui_ScriptAdvancedPrompt->resize(452, 476);

        buttonLayout = new QHBoxLayout();
        buttonLayout->setObjectName(QString::fromUtf8("buttonLayout"));
        buttonLayout->setSizeConstraint(QLayout::SetDefaultConstraint);

        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);

        okButton = new QPushButton(Ui_ScriptAdvancedPrompt);
        okButton->setText(QString::fromUtf8(ok));
        okButton->setObjectName(QString::fromUtf8("okButton"));
        
        buttonLayout->addWidget(okButton);

        QObject::connect(okButton, SIGNAL(clicked()), Ui_ScriptAdvancedPrompt, SLOT(close()));

        okButton->setDefault(true);


        QMetaObject::connectSlotsByName(Ui_ScriptAdvancedPrompt);
    } // setupUi

    void retranslateUi(QDialog *Ui_ScriptAdvancedPrompt)
    {
        // ScriptAdvancedPrompt->setWindowTitle(QApplication::translate("ScriptAdvancedPrompt", "About Tiled", nullptr));
        // logo->setText(QString());
        // donateButton->setText(QApplication::translate("ScriptAdvancedPrompt", "Donate \342\206\227", nullptr));
        // okButton->setText(QApplication::translate("ScriptAdvancedPrompt", "OK", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ScriptAdvancedPrompt: public Ui_ScriptAdvancedPrompt {};
} // namespace Ui

class ScriptAdvancedPrompt: public QDialog, private Ui::ScriptAdvancedPrompt
{
    Q_OBJECT

public:
    ScriptAdvancedPrompt(QWidget *parent = nullptr);

// private:
};
QT_END_NAMESPACE
