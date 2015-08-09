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


#ifndef PATHELEMENT_H
#define PATHELEMENT_H

#include "model.h"

#include <QObject>
#include <QGraphicsPathItem>
#include <QItemSelectionModel>
#include <QModelIndex>
#include <QBasicTimer>

class PathElement : public QGraphicsPathItem, public QObject
{
public:
    // Element type to adjust colour and pens accordingly
    enum ElementType { Edge, EdgeNoShape, NormalLane, IntLane, PlainJunction, IntJunction, Connection };

    // Constructor
    PathElement(ElementType type, QString shape, Model *model, Item *item, QItemSelectionModel *selectionModel);

    // Reimplementation of the shape method for more accurate selection
    QPainterPath shape() const;
    QPainterPath centerLine() const;
    QRectF boundingRect() const;

    // Index of the Item in the model (tree view)
    QModelIndex modelIndex;

    // Selection and deselection methods
    void select();
    void deselect();
    bool isSelected() const;

    // Changes an element property
    void switchState(Model::ElementProperty prop, bool state);

    // Highlights the whole element or point
    void highlight();
    void highlightPoint(int node);
    
    // deletes the selected element
    void deleteElement();

    // MW: just for testing purposes
    ElementType type;
    
    // getter function for item
    Item* getItem();

    // Pointer to the model
    Model *model;
    
protected:
    // Calls a selection change in the Selection Model (to update the tree and properties view)
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

    // Reimplemented paint method
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    // Timer event used when highlighting the element
    void timerEvent(QTimerEvent *event);

    // Context menu call
    void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
    // General properties
    int r, g, b;
    Qt::PenStyle style;
    bool selected, fill, isWired, editable, showArrow;
    int selectedNode;
    qreal normalW, wireW, z;
    qreal gripRadius;

    // Element type
    //ElementType type;

    // Path nodes
    QList <QPointF> nodes;

    // Pointer to the model item this graphic item belongs to
    Item* item;
    
    // Pointer to the Selection Model, to call it when the item is clicked
    QItemSelectionModel *selectionModel;

    // Center path and border path
    QPainterPath centerPath, borderPath, arrow;

    // Calculates the paths from the current node positions
    void calcPaths();

    // Returns unit vector for two given points, used in the path calculations
    QPointF unitVector(QPointF pA, QPointF pB, bool perpendicular) const;

    // Parses a string with coordinates and fills in the list of nodes
    void parseShape(QString shape);

    // Point used in mouse movement events
    QPointF lastPos;

    // Updates the shape and length properties in XML domDocument after the nodes have been modified
    void updateXML();

    // Returns a string with the nodes' coordinates; used by updateXML()
    QString shapePoints() const;

    // Returns a string with the shape length; used by updateXML()
    QString length() const;

    // Context menu actions
    void insertNode(QPointF I);
    void deleteNode();
    void copyID();
    void copyFirstNode();
    void copyLastNode();
    void pasteFirstNode();
    void pasteLastNode();

    // Timer and properties used for highlighting
    QBasicTimer timer;
    int blink, blinkingNode;
};

#endif // PATHELEMENT_H
