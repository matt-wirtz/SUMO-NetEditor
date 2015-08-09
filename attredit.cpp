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


#include "attredit.h"

AttrEdit::AttrEdit(const QString &text, QWidget *parent) : QHBoxLayout(parent)
{
    // Each AttrEddit is a QHBoxLayout with a LineEdit on the left and a ComboBox
    // on the right; depending on the attribute, either one or the other is shown.

    // Create LineEdit and Combo, add them to this layout and by default hide Combo
    ledit = new QLineEdit(text);
    combo = new QComboBox();
    addWidget(ledit);
    addWidget(combo);
    combo->hide();
    isCombo = false;

    // Keep a copy of the style sheet to restore the background colour when necessary
    defaultStyleSheet = ledit->styleSheet();

    // Connect to allow changing background colour when text is modified, and to update changes in the XML domDocument
    connect(ledit, SIGNAL(textEdited(QString)), this, SLOT(onTextChanged(QString)));
    connect(ledit, SIGNAL(returnPressed()), this, SLOT(acceptChangedText()));
    connect(combo, SIGNAL(activated(QString)), this, SLOT(acceptChangedCombo(QString)));
    textEdited = false;
}

void AttrEdit::onTextChanged(QString text)
{
    Q_UNUSED(text);

    // Set background colour to khaki
    ledit->setStyleSheet("background-color: rgb(189,183,107)");

    // Update statusBar
    emit statusUpdate(tr("Hit Enter/Return to confirm changes or select another element to abandon them"));

    textEdited = true;
}

void AttrEdit::setAttribute(QString elementType, QString attr, QString value, bool enabled)
{
    // Reassign values for the AttrEdit
    this->attr = attr;
    this->value = value;

    // Decide whether to show the LineEdit or the ComboBox
    // Based on net_file.xsd schema in SUMO 0.21.0
    QStringList options;
    isCombo = true;
    if (elementType == "edge" && attr == "function")
        options << "normal" << "internal" << "connector" << "crossing" << "walkingarea";
    else if (elementType == "junction" && attr == "type")
        options << "traffic_light" << "right_before_left" << "priority" << "dead_end" << "unregulated"
                << "traffic_light_unregulated" << "allway_stop" << "priority_stop" << "internal";
    else if (elementType == "junction" && attr == "type")
        options << "traffic_light" << "right_before_left" << "priority" << "dead_end" << "unregulated"
                << "traffic_light_unregulated" << "allway_stop" << "priority_stop" << "internal";
    else if (elementType == "connection" && attr == "dir")
        options << "s" << "t" << "l" << "r" << "L" << "R";
    else if (elementType == "connection" && attr == "state")
        options << "M" << "m" << "o" << "=" << "-" << "s" << "w";
    else
        isCombo = false;

    // Show Combo or LineEdit accordingly
    if (isCombo)
    {
        combo->clear();
        combo->addItems(options);
#if QT_VERSION >= 0x040000
        int index = combo->findText(value);
        combo->setCurrentIndex(index);
#elif QT_VERSION >= 0x050000
        combo->setCurrentText(value);
#endif
        combo->setEnabled(enabled);
        combo->show();
        ledit->hide();
    }
    else
    {
        ledit->setText(value);
        ledit->setStyleSheet(defaultStyleSheet);
        ledit->setEnabled(enabled);
        ledit->show();
        combo->hide();
        textEdited = false;
    }
}

void AttrEdit::setModelAndItem(Model *model, Item *item)
{
    this->model = model;
    this->item = item;
}

void AttrEdit::acceptChangedText()
{
    if (textEdited)
    {
        // Set background colour to green
        ledit->setStyleSheet("background-color: rgb(51,204,51)");

        // Update attribute in XML domDocument
        value = ledit->text();
        model->editAttribute(item->xmlNode, item->xmlSubNode, attr, value);

        // Update statusBar
        emit statusUpdate(tr("Ready"));

        textEdited = false;
    }
}

void AttrEdit::acceptChangedCombo(QString newValue)
{
    if (newValue != value)
    {
        // Update attribute in XML domDocument
        value = newValue;
        model->editAttribute(item->xmlNode, item->xmlSubNode, attr, value);
    }
}

void AttrEdit::hide()
{
    // Hide both the LineEdit and the ComboBox
    ledit->hide();
    combo->hide();
}
