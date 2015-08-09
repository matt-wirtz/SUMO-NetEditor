/***************************************************************************
 *  Copyright (c) 2014 Martin Llavallol <m5lmodelling@gmail.com>           *
 *                                                                         *
 *  This file is part of Network Editor for SUMO.                          *
 *                                                                         *
 *  Network Editor for SUMO is free software: you can redistribute it      *
 *  and/or modify it under the terms of the GNU General Public License     *
 *  as published by the Free Software Foundation, either version 3 of      *
 *  the License, or (at your option) any later version.                    *
 *                                                                         *
 *  Network Editor for SUMO is distributed in the hope that it will be     *
 *  useful but WITHOUT ANY WARRANTY; without even the implied warranty     *
 *  of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the       *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *  You should have received a copy of the GNU General Public License      *
 *  along with Network Editor for SUMO.  If not, see                       *
 *  <http://www.gnu.org/licenses/>.                                        *
 ***************************************************************************/


#ifndef ATTREDIT_H
#define ATTREDIT_H

#include "model.h"
#include "item.h"

#include <QHBoxLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QKeyEvent>

class AttrEdit : public QHBoxLayout
{
    Q_OBJECT

public:
    // Constructor
    AttrEdit(const QString &text, QWidget *parent = 0);

    // Shows the LineEdit or ComboBox when a new element is selected and resets them;
    // called by EditView::selectionChanged
    void setAttribute(QString elementType, QString attr, QString value, bool enabled);

    // Set references to the model and the item clicked
    void setModelAndItem(Model *model, Item *item);

    // Hide function
    void hide();

private slots:
    // Slot to change background colour and send message to the status bar
    void onTextChanged(QString text);

    // Store changes in the text or combo
    void acceptChangedText();
    void acceptChangedCombo(QString newValue);

signals:
    // Updates status bar
    void statusUpdate(QString msg);

private:
    // Line edit and Combo box
    QLineEdit *ledit;
    QComboBox *combo;

    // Boolean store whether the attribute requires to show a Combo or a LineEdit
    bool isCombo;

    // Pointers to the model and the current selected item
    Model *model;
    Item *item;

    // Default style sheet to restore background colour
    QString defaultStyleSheet;

    // attr = xml attribute that is edited in this LineEdit
    // value = stores the default value, in case Esc is pressed
    QString attr, value;

    // True if the text has been modified
    bool textEdited;
};

#endif // ATTREDIT_H
