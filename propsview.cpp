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


#include "propsview.h"
#include "item.h"
#include "model.h"

PropsView::PropsView(QWidget *parent) : QWidget(parent)
{
    // Create layout
    layout = new QFormLayout();

    // Append QLabel items to the 'field' list and add them to the layout in row of two
    for (int i = 0; i < 12; ++i)
    {
        field.append(new QLabel(""));
        field.last()->setWordWrap(true);
        field.append(new QLabel(""));
        field.last()->setWordWrap(true);
        connect(field.last(), SIGNAL(linkActivated(QString)), this, SLOT(linkClicked(QString)));
        layout->addRow(field[2 * i], field[2 * i + 1]);
    }
    setLayout(layout);

    // Establish the attributes for each element type
    // Based on xsd schema in SUMO 0.20.0
    edgeAttr << "id" << "function" << "from" << "to" << "name" << "priority"
             << "length" << "type" << "shape" << "spreadType";

    laneAttr << "id" << "index" << "allow" << "disallow" << "prefer"
             << "speed" << "length" << "endOffset" << "width" << "shape";

    juncAttr << "id" << "x" << "y" << "z" << "type" << "incLanes" << "intLanes" << "shape";

    connAttr << "from" << "fromLane" << "to" << "toLane" << "pass"
             << "via" << "tl" << "linkIndex" << "dir" << "state";

    tlLgcAttr << "id" << "type" << "programID" << "offset";

    reqAttr << "index" << "response" << "foes" << "cont";

    phaseAttr << "duration" << "state";
}

void PropsView::selectionChanged(QItemSelection on, QItemSelection off)
{
    Q_UNUSED(off);

    // Select the new item
    if (!on.indexes().empty())
    //if (on.indexes().count() > 0)
    {
        if (on.indexes()[0].isValid())
        {
            // Find selected item pointer
            Item *itemOn = static_cast<Item*>(on.indexes()[0].internalPointer());

            // If the selected item is of the first level, clear all fields
            if (model->isCaption(itemOn))
            {
                for (int i = 0; i < field.length(); ++i)
                    field[i]->setText("");
            }
            else
            {
                // Retrieve element from the XML domDocument
                QDomElement element = model->getXMLelement(itemOn->xmlNode, itemOn->xmlSubNode);

                // Update fields / QLabels
                int i;
                QString elementName, attr;
                QStringList attrList;

                switch (itemOn->type)
                {
                    case Item::Edge:        elementName = "edge";       attrList = edgeAttr;    break;
                    case Item::Lane:        elementName = "lane";       attrList = laneAttr;    break;
                    case Item::Junction:    elementName = "junction";   attrList = juncAttr;    break;
                    case Item::Connection:  elementName = "connection"; attrList = connAttr;    break;
                    case Item::tlLogic:     elementName = "tlLogic";    attrList = tlLgcAttr;   break;
                    case Item::Request:     elementName = "request";    attrList = reqAttr;     break;
                    case Item::Phase:       elementName = "phase";      attrList = phaseAttr;   break;
                }

                field[0]->setText("element");
                field[1]->setText(elementName);
                for (i = 0; i < attrList.length(); ++i)
                {
                    field[2 * i + 2]->setText(attrList[i]);
                    attr = element.attribute(attrList[i]);
                    if (elementName == "junction" && attrList[i] == "incLanes")
                        field[2 * i + 3]->setText(hyperlink("3/", element.attribute(attrList[i])));
                    else if (elementName == "junction" && attrList[i] == "intLanes")
                        field[2 * i + 3]->setText(hyperlink("4/", element.attribute(attrList[i])));
                    else if (elementName == "connection" && attrList[i] == "fromLane")
                        field[2 * i + 3]->setText(hyperlink("9/" + element.attribute("from") + "_", element.attribute(attrList[i])));
                    else if (elementName == "connection" && attrList[i] == "toLane")
                        field[2 * i + 3]->setText(hyperlink("9/" + element.attribute("to") + "_", element.attribute(attrList[i])));
                    else if (elementName == "connection" && attrList[i] == "via")
                        field[2 * i + 3]->setText(hyperlink("4/", element.attribute(attrList[i])));
                    else if (elementName == "edge" && attrList[i] == "from")
                        field[2 * i + 3]->setText(hyperlink("1/", element.attribute(attrList[i])));
                    else if (elementName == "edge" && attrList[i] == "to")
                        field[2 * i + 3]->setText(hyperlink("1/", element.attribute(attrList[i])));

                    else if (elementName == "junction" && attrList[i] == "shape")
                        field[2 * i + 3]->setText(hyperlink("5/" + element.attribute("id"), element.attribute(attrList[i])));
                    else if (elementName == "edge" && attrList[i] == "shape")
                        field[2 * i + 3]->setText(hyperlink("6/" + element.attribute("id"), element.attribute(attrList[i])));
                    else if (elementName == "lane" && attrList[i] == "shape")
                        field[2 * i + 3]->setText(hyperlink("7/" + element.attribute("id"), element.attribute(attrList[i])));

                    else
                        field[2 * i + 3]->setText(element.attribute(attrList[i]));
                }
                for (int j = i * 2 + 2; j < 22; ++j)
                    field[j]->setText("");
            }
        }
    } else {
        // nothing selected: happens when an item was deleted
        // show a cleared props view window with all fields emtpy
        for (int i = 0; i < field.length(); ++i)
            field[i]->setText("");
    }

}

QString PropsView::hyperlink(QString prefix, QString attrString)
{
    // Create hyperlink
    QString finalString = "";
    QStringList tokens = attrString.simplified().split(" ");
    bool points = (prefix.left(2) == "5/" || prefix.left(2) == "6/" || prefix.left(2) == "7/" ? true : false);

    for (int i = 0; i < tokens.size(); ++i)
        if (points)
            finalString += "<a href=\"" + prefix + "/" + QString::number(i) + "\">" + tokens[i] + "</a> ";
        else
            finalString += "<a href=\"" + prefix + tokens[i] + "\">" + tokens[i] + "</a> ";

    return finalString;
}

void PropsView::linkClicked(QString link) const
{
    // Send request to the model
    model->highlightHyperlink(link);
}
