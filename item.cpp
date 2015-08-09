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


#include "item.h"

Item::Item(QString name, int iconType, Item *parent)
{
    // Initialise the class
    this->parentItem = parent;
    this->name = name;
    this->iconType = iconType;
    xmlLine = -1;
    xmlNode = -1;
    xmlSubNode = -1;
    graphicItem1 = NULL;
    graphicItem2 = NULL;
    hasPath = false;
    hasPoint = false;
}

Item::~Item()
{
    //references.clear();

    // Delete tree
    qDeleteAll(childItems);
}

void Item::setXMLdata(XMLElement type, int line, int node, int subnode)
{
    // Update the xml data from the domDocument
    this->type = type;
    this->xmlLine = line;
    this->xmlNode = node;
    this->xmlSubNode = subnode;
}

Item *Item::parent() const
{
    // Implementation required by QAbstractItemModel
    return parentItem;
}

Item *Item::child(int i)
{
    // Implementation required by QAbstractItemModel
    return childItems.value(i);
}

Item *Item::child(QString key)
{
    // Ovedload to get faster access to a child based on the child id
    // using the references QMultiHash
    if (references.contains(key))
        return references.value(key);
    return NULL;
}

Item *Item::operator[] (int i)
{
    // Operator overload to reduce sintaxis
    return childItems.value(i);
}

Item *Item::operator[] (QString key)
{
    // Operator overload to reduce sintaxis
    if (references.contains(key))
        return references.value(key);
    return NULL;
}

int Item::appendChild(Item *item)
{
    // Add a child to the tree
    childItems.append(item);

    // Create the id reference in the QMultiHash for faster access
    references.insert(item->name, item);

    // Returns the child number
    return (childItems.count() - 1);
}

void Item::removeChild(Item *item)
{
    // Remove child from the childItems QList of childs
    // better to use the QMultiHash for direct access
    int i = 0;
    
    if ( childItems.count() > 0 ) {
        while ( i < childItems.count() ) {
            if ( childItems[i]->name == item->name) {
                childItems.removeAt(i);
            } else {
                i++;
            }
        }
    }
}

int Item::row() const
{
    // Implementation required by QAbstractItemModel
    if (parentItem) return
        parentItem->childItems.indexOf(const_cast<Item*>(this));
    return 0;
}

int Item::childCount() const
{
    // Implementation required by QAbstractItemModel
    return childItems.count();
}
