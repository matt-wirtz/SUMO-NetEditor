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


#ifndef POINTELEMENT_H
#define POINTELEMENT_H

#include "model.h"

#include <QObject>
#include <QGraphicsEllipseItem>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QBasicTimer>

class PointElement : public QGraphicsEllipseItem, public QObject
{
public:
    // Element type to adjust colour and pens accordingly
    enum ElementType { PlainJunction, IntJunction, Connection };

    // Constructor
    PointElement(ElementType type, qreal x, qreal y, Model *model, Item *item, QItemSelectionModel *selectionModel);

    // Index of the Item in the model (tree view)
    QModelIndex modelIndex;

    // Selection and deselection methods
    void select();
    void deselect();
    bool isSelected() const;

    // Changes an element property
    void switchState(Model::ElementProperty prop, bool state);

    // Highlights the element
    void highlight();

    // deletes the selected element
    void deleteElement();
    
    // MW: Element type
    ElementType type;
    
    // getter function for item
    Item* getItem();

    // Pointer to the model
    Model *model;

protected:
    // Calls a selection change in the Selection Model (to update the tree and properties view)
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *);

    // Timer event used when highlighting the element
    void timerEvent(QTimerEvent *event);

    // Context menu call
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    // General properties
    int r, g, b;
    bool selected, editable, moving;
    qreal x, y, radius, w, z;

    // Pointer to the model item this graphic item belongs to
    Item* item;

    // Pointer to the Selection Model, to call it when the item is clicked
    QItemSelectionModel *selectionModel;

    // Point used in mouse movement events
    QPointF lastPos;

    // Updates the x and y properties in XML domDocument after the nodes have been modified
    void updateXML();

    // Context menu actions
    void copyCoords();
    void pasteCoords();

    // Timer and properties used for highlighting
    QBasicTimer timer;
    int blink;
};

#endif // POINTELEMENT_H
