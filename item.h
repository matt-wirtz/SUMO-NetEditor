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


#ifndef ITEM_H
#define ITEM_H

#include "pathelement.h"
#include "pointelement.h"

#include <QList>
#include <QMultiHash>
#include <QDomNode>

class Item
{
public:
    // Constructor and destructor
    Item(QString name, int iconType, Item *parent = 0);
    ~Item();

    // Type of element, based on the xml tag, used to display the right set of attributes for the element in the properties view
    enum XMLElement { Edge, Lane, Junction, Connection, tlLogic, Request, Phase };

    // Item relationships, used by Model class
    Item *child(int i);
    Item *child(QString key);
    Item *operator[] (int i);
    Item *operator[] (QString key);
    Item *parent() const;
    int appendChild(Item *item);
    void removeChild(Item *item);
    int childCount() const;
    int row() const;

    // Item properties related to the XML element and the location of the element within the XML document
    // type = xml tag
    // name = attribute "id"
    // iconType = used by Model::data()
    // xmlLine = line of the xml element within the file
    // xmlNode = node number within <net> branch
    // xmlSubNode = node number within the <edge>, <junction> or <tllogic> branch (this only applies to <lane> elements)
    XMLElement type;
    QString name;
    int iconType, xmlLine, xmlNode, xmlSubNode;

    // Helps setting a few properties in one go
    void setXMLdata(XMLElement type, int line, int node, int subnode);

    // Pointers to graphic items in the Graphics Scene
    // graphicItem1 is always a Path Element - used for edges, lanes, junction polygons and connections
    // graphicItem2 is always a Point Element - used for junction XY points and for connections that go from and to the same point
    bool hasPath;
    PathElement *graphicItem1;
    bool hasPoint;
    PointElement *graphicItem2;

    // Point used for edges that do not have a 'shape' property, generated from the 'x' and 'y' properties of a <junction>
    QString junctionXY;

private:
    // Pointer to parent item
    Item *parentItem;

    // List of children
    QList<Item*> childItems;

    // Reference container to children
    QMultiHash<QString, Item*> references;
};

#endif // ITEM_H
