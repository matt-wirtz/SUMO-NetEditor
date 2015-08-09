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


#include "editview.h"
#include "item.h"
#include "model.h"

#include <QStatusBar>

EditView::EditView(QMainWindow *parent) : QWidget(parent)
{
    // Create layout
    layout = new QFormLayout();

    // Append blank QLabels in the field QList and AttrEdits in the edit QList for 11 attributes
    // and add them in the form layout
    for (int i = 0; i < 12; ++i)
    {
        field.append(new QLabel(""));
        field.last()->setWordWrap(true);
        field.last()->hide();
        edit.append(new AttrEdit(""));
        connect(edit.last(), SIGNAL(statusUpdate(QString)), parent->statusBar(), SLOT(showMessage(QString)));
        edit.last()->hide();
        layout->addRow(field[i], edit[i]);
    }
    setLayout(layout);

    // Establish the attributes for each element type
    // Based on net_file.xsd schema in SUMO 0.20.0
    edgeAttr << "id" << "function" << "from" << "to" << "name" << "priority"
             << "length" << "type" << "shape" << "spreadType";
    edgeEdit << false << false << true << true << true << true
             << true << true << true << true;

    laneAttr << "id" << "index" << "allow" << "disallow" << "prefer"
             << "speed" << "length" << "endOffset" << "width" << "shape";
    laneEdit << false << false << true << true << true
             << true << true << true << true << false;

    juncAttr << "id" << "x" << "y" << "z" << "type" << "incLanes" << "intLanes" << "shape";
    juncEdit << false << false << false << true << false << true << true << false;

    connAttr << "from" << "fromLane" << "to" << "toLane" << "pass"
             << "via" << "tl" << "linkIndex" << "dir" << "state";
    connEdit << true << true << true << true << true
             << true << true << true << true << true;

    tlLgcAttr << "id" << "type" << "programID" << "offset";
    tlLgcEdit << false << true << true << true;

    reqAttr << "index" << "response" << "foes" << "cont";
    reqEdit << true << true << true << true;

    phaseAttr << "duration" << "state";
    phaseEdit << true << true;
}

void EditView::selectionChanged(QItemSelection on, QItemSelection off)
{
    Q_UNUSED(off);

    // Select the new item
    if (on.indexes().count() > 0)
        if (on.indexes()[0].isValid())
        {
            // Find selected item pointer
            Item *itemOn = static_cast<Item*>(on.indexes()[0].internalPointer());

            // If the selected item is item caption, clear all fields
            if (model->isCaption(itemOn))
            {
                for (int i = 0; i < field.length(); ++i)
                    field[i]->hide();
                for (int i = 0; i < edit.length(); ++i)
                    edit[i]->hide();
            }
            else
            {
                // Retrieve element from the XML domDocument
                QDomElement element = model->getXMLelement(itemOn->xmlNode, itemOn->xmlSubNode);

                // Update fields
                int i;
                QString elementName;
                QStringList attrList;
                QList<bool> editable;

                // According to the element type, find the name, the attributese list and which of them can be edited
                switch (itemOn->type)
                {
                    case Item::Edge:        elementName = "edge";       attrList = edgeAttr;    editable = edgeEdit;    break;
                    case Item::Lane:        elementName = "lane";       attrList = laneAttr;    editable = laneEdit;    break;
                    case Item::Junction:    elementName = "junction";   attrList = juncAttr;    editable = juncEdit;    break;
                    case Item::Connection:  elementName = "connection"; attrList = connAttr;    editable = connEdit;    break;
                    case Item::tlLogic:     elementName = "tlLogic";    attrList = tlLgcAttr;   editable = tlLgcEdit;   break;
                    case Item::Request:     elementName = "request";    attrList = reqAttr;     editable = reqEdit;     break;
                    case Item::Phase:       elementName = "phase";      attrList = phaseAttr;   editable = phaseEdit;   break;
                }

                // Update the field labels and the AttrEdits
                field[0]->setText("element");
                field[0]->show();
                edit[0]->setAttribute("", "", elementName, false);
                for (i = 0; i < attrList.length(); ++i)
                {
                    field[i + 1]->setText(attrList[i]);
                    field[i + 1]->show();
                    edit[i + 1]->setAttribute(elementName, attrList[i], element.attribute(attrList[i]), editable[i]);
                    edit[i + 1]->setModelAndItem(model, itemOn);
                }
                // Hide the fields and AttrEdits that are not needed for the selected item
                for (int j = i + 1; j < field.length(); ++j)
                {
                    field[j]->hide();
                    edit[j]->hide();
                }
            }
        }
}
