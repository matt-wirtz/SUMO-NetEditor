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


#include "pathelement.h"
#include "item.h"

#include <QPen>
#include <QBrush>
#include <QPainter>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPointF>
#include <qmath.h>
#include <QCursor>
#include <QMenu>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QDebug>

PathElement::PathElement(ElementType type, QString shape, Model *model, Item *item, QItemSelectionModel *selectionModel)
{
    // Initialise members
    parseShape(shape);
    this->type = type;
    this->model = model;
    this->item = item;
    this->selectionModel = selectionModel;

    // Set element properties
    switch (type)
    {
        case Edge:
            r = 255; g = 192; b = 0;   normalW = 1.5;  wireW = 0.5;  z = 3; fill = false; style = Qt::DashLine;   break;
        case EdgeNoShape:
            r = 255; g = 192; b = 0;   normalW = 1.5;  wireW = 0.5;  z = 3; fill = false; style = Qt::DotLine;    break;
        case NormalLane:
            r = 169; g = 169; b = 169; normalW = 3.3;  wireW = 0.5;  z = 1; fill = false; style = Qt::SolidLine;  break;
        case IntLane:
            r = 0;   g = 0;   b = 139; normalW = 3.3;  wireW = 0.5;  z = 2; fill = false; style = Qt::SolidLine;  break;
        case PlainJunction:
            r = 0;   g = 100; b = 0;   normalW = 0.2;  wireW = 0.2;  z = 5; fill = true;  style = Qt::SolidLine;  break;
        case IntJunction:
            r = 128; g = 0;   b = 128; normalW = 0.2;  wireW = 0.2;  z = 5; fill = true;  style = Qt::SolidLine;  break;
        case Connection:
            r = 255; g = 255; b = 0;   normalW = 0.15; wireW = 0.15; z = 4; fill = false; style = Qt::SolidLine;  break;
    }
    setZValue(z);

    // Initialise more members
    isWired = false;
    selected = false;
    editable = false;
    showArrow = false;
    gripRadius = 0.6;
    blink = -1;
    blinkingNode = -1;
    selectedNode = -1;

    // Calculate paths and create QGraphicsPathItem
    calcPaths();
    QGraphicsPathItem(centerPath);

    // Set arrow cursor
    setCursor(QCursor(Qt::ArrowCursor));

    //update();
}

void PathElement::select()
{
    // Set selected as true, bring element to front and redraw
    selected = true;
    setZValue(10);
    update();
}

void PathElement::deselect()
{
    // Set selected as false, put element back into its normal position and redraw
    selected = false;
    setZValue(z);
    update();
}

bool PathElement::isSelected() const
{
    return selected;
}

void PathElement::switchState(Model::ElementProperty prop, bool state)
{
    // Change element property
    switch (prop)
    {
    case Model::ViewElement:
        if (state) show(); else hide();
        break;
    case Model::EditElement:
        editable = state;
        if (selected) update();
        break;
    case Model::WireElement:
        isWired = state;
        calcPaths();
        if (selected) select(); else deselect();
        break;
    case Model::ArrowElement:
        showArrow = state;
        update();
        break;
    }
}

void PathElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // Select a node if within the gripRadius of one
    selectedNode = -1;
    if (selected && editable)
    {
        qreal distance;
        for (int i = 0; i < nodes.count(); ++i)
        {
            distance = qSqrt(qPow(nodes[i].x() - event->pos().x(), 2) + qPow(nodes[i].y() - event->pos().y(), 2));
            if (distance < gripRadius) {
                selectedNode = i;
            }
        }
        lastPos = event->pos();
        update();
    }

    // Select the element in the tree and trigger all the selection processes
    if (selectionModel)
        selectionModel->select(modelIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void PathElement::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Move the node if previously clicked on one, and update the geometry
    if (selectedNode > -1)
    {
        nodes[selectedNode] += event->pos() - lastPos;
        lastPos = event->pos();

        // Update center and border paths
        prepareGeometryChange();
        calcPaths();
        update();
    }
}

void PathElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    // Update the XML domDocument if the node was changed
    if (selectedNode > -1)
    {
        selectedNode = -1;
        updateXML();
        update();
    }
}

void PathElement::paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    // Determine if paint colour is red (for selected) or normal
    QColor colour = (selected ? QColor(255, 0, 0) : QColor(r, g, b));
    painter->setPen(QPen(colour, (isWired ? wireW : normalW), style, Qt::FlatCap, Qt::BevelJoin));

    // Determine fill brush if required
    if (fill) painter->setBrush(QBrush((selected ? QColor(255, 0, 0, 100) : QColor(r, g, b, 100))));

    // Draw the center path
    painter->drawPath(centerPath);

    if (editable && selected)
    {
        // Draw border path (only for debugging purposes)
        //painter->setPen(QPen(Qt::black, 0.05));
        //painter->setBrush(Qt::NoBrush);
        //painter->drawPath(shape());

        // Draw black nodes
        painter->setPen(Qt::NoPen);
        painter->setBrush(Qt::black);
        for (int i = 0; i < nodes.count(); ++i)
            painter->drawEllipse(nodes[i], gripRadius, gripRadius);

        // Draw selected node
        if (selectedNode > -1)
        {
            painter->setBrush(QColor(192, 0, 0));
            painter->drawEllipse(nodes[selectedNode], gripRadius, gripRadius);
        }
    }

    // Draw arrow
    if (showArrow && type != PlainJunction && type != IntJunction)
    {
        painter->setPen(QPen(selected ? QColor(192, 0, 0) : Qt::black, 0.1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->setBrush(selected ? QColor(192, 0, 0) : Qt::black);
        painter->drawPath(arrow);
    }

    // Draw blinking node
    if (blinkingNode >= 0 && blink % 2 == 1)
    {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(192, 0, 0));
        painter->drawEllipse(nodes[blinkingNode], gripRadius, gripRadius);
    }
}

QPainterPath PathElement::shape() const
{
    // Implementation required by QGraphicsItem
    return borderPath;
}

QPainterPath PathElement::centerLine() const
{
    // Returns center path for junction and tlLogic editors
    return centerPath;
}

QRectF PathElement::boundingRect() const
{
    // Implementation required by QGraphicsItem
    return borderPath.boundingRect();
}

void PathElement::calcPaths()
{
    // Update center path
    centerPath = QPainterPath();
    centerPath.moveTo(nodes[0]);
    for (int i = 1; i < nodes.count(); ++i)
        centerPath.lineTo(nodes[i]);
    if (type == PlainJunction) centerPath.closeSubpath();

    // Update border path
    borderPath = QPainterPath();
    qreal r = (isWired ? wireW : normalW) / 2;
    if (r < 0.7) r = 0.7;

    QPointF segmentOffset;
    bool first = true;

    if (type != PlainJunction)
    {
        // Offset one side of the path
        for (int i = 0; i < nodes.count() - 1; ++i)
        {
            segmentOffset = unitVector(nodes[i], nodes[i + 1], true) * r;
            if (first)
            {
                borderPath.moveTo(nodes[i] + segmentOffset);
                first = false;
            }
            borderPath.lineTo(nodes[i] + segmentOffset);
            borderPath.lineTo(nodes[i + 1] + segmentOffset);
        }

        // Extended end
        QPointF extension = unitVector(nodes[nodes.count() - 2], nodes.last(), false) * r;
        borderPath.lineTo(nodes.last() + segmentOffset + extension);
        borderPath.lineTo(nodes.last() - segmentOffset + extension);

        // Offset other side of the path
        for (int i = nodes.count() - 1; i > 0; --i)
        {
            segmentOffset = unitVector(nodes[i], nodes[i - 1], true) * r;
            borderPath.lineTo(nodes[i] + segmentOffset);
            borderPath.lineTo(nodes[i - 1] + segmentOffset);
        }

        // Extended end
        extension = unitVector(nodes[1], nodes[0], false) * r;
        borderPath.lineTo(nodes.first() + segmentOffset + extension);
        borderPath.lineTo(nodes.first() - segmentOffset + extension);
    }
    else
    {
        // Offset one side of the path
        for (int i = 0; i < nodes.count() - 1; ++i)
        {
            segmentOffset = unitVector(nodes[i], nodes[i + 1], true) * r;
            if (first)
            {
                borderPath.moveTo(nodes[i] + segmentOffset);
                first = false;
            }
            borderPath.lineTo(nodes[i] + segmentOffset);
            borderPath.lineTo(nodes[i + 1] + segmentOffset);
        }

        // Last segment
        segmentOffset = unitVector(nodes.last(), nodes[0], true) * r;
        borderPath.lineTo(nodes.last() + segmentOffset);
        borderPath.lineTo(nodes[0] + segmentOffset);
    }
    borderPath.closeSubpath();

    // Update direction arrow
    if (type != PlainJunction && type !=  IntJunction)
    {
        QPointF direction, directionP, middle;
        middle = centerPath.pointAtPercent(0.50);
        direction = unitVector(centerPath.pointAtPercent(0.49), centerPath.pointAtPercent(0.51), false);
        directionP = unitVector(centerPath.pointAtPercent(0.49), centerPath.pointAtPercent(0.51), true);
        arrow = QPainterPath();
        arrow.moveTo(middle + direction);
        arrow.lineTo(middle - direction + 0.8 * directionP);
        arrow.lineTo(middle - direction - 0.8 * directionP);
        arrow.lineTo(middle + direction);
        arrow.closeSubpath();
    }
}

QPointF PathElement::unitVector(QPointF pA, QPointF pB, bool perpendicular) const
{
    // Calculates the unit vector (or its perpendicular) given two points
    qreal xu, yu;
    qreal norm = qSqrt(qPow(pB.x() - pA.x(), 2) + qPow(pB.y() - pA.y(), 2));
    if (norm > 0)
    {
        xu = (pB.x() - pA.x()) / norm;
        yu = (pB.y() - pA.y()) / norm;
    }
    else
        xu = yu = 0;

    if (perpendicular)
        return QPointF(-yu, xu);
    else
        return QPointF(xu, yu);
}

void PathElement::parseShape(QString shape)
{
    // Replace spaces for commas, and split the string on the commas
    shape.replace(" ", ",");
    QStringList tokens = shape.split(",");

    // Fill in nodes with the values from the stringlist
    nodes.append(QPointF(tokens[0].toDouble(), tokens[1].toDouble()));
    for (int j = 2; j < tokens.size(); j += 2)
        nodes.append(QPointF(tokens[j].toDouble(), tokens[j + 1].toDouble()));
}

QString PathElement::shapePoints() const
{
    // Returns a string with the nodes coordinates
    QString text = QString::number(nodes[0].x(), 'f', 2) + QString(",") + QString::number(nodes[0].y(), 'f', 2);
    for (int i = 1; i < nodes.count(); ++i)
        text += QString(" ") + QString::number(nodes[i].x(), 'f', 2) + QString(",") + QString::number(nodes[i].y(), 'f', 2);
    return text;
}

QString PathElement::length() const
{
    // Returns the length of the center path in a string format
    qreal l = 0;
    for (int i = 1; i < nodes.count(); ++i)
        l += qSqrt(qPow(nodes[i].x() - nodes[i - 1].x(), 2) + qPow(nodes[i].y() - nodes[i - 1].y(), 2));

    return QString::number(l, 'f', 2);
}

void PathElement::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // Create context menu
    QMenu menu;
    QAction *insertNodeAction = menu.addAction(QObject::tr("Insert node"));
    QAction *deleteNodeAction = menu.addAction(QObject::tr("Delete node"));
    menu.addSeparator();
    QAction *copyFirstNodeAction = menu.addAction(QObject::tr("Copy first node"));
    QAction *copyLastNodeAction = menu.addAction(QObject::tr("Copy last node"));
    QAction *pasteFirstNodeAction = menu.addAction(QObject::tr("Paste first node"));
    QAction *pasteLastNodeAction = menu.addAction(QObject::tr("Past last node"));
    QAction *copyIDAction = menu.addAction(QObject::tr("Copy element id"));
    menu.addSeparator();
    QAction *deleteElementAction = menu.addAction(QObject::tr("Delete element"));

    if (!editable)
    {
        insertNodeAction->setEnabled(false);
        pasteFirstNodeAction->setEnabled(false);
        pasteLastNodeAction->setEnabled(false);
        deleteElementAction->setEnabled(false);
        deleteNodeAction->setEnabled(false);
    } else {
        qreal distance;
        for (int i = 0; i < nodes.count(); ++i)
        {
            distance = qSqrt(qPow(nodes[i].x() - event->pos().x(), 2) + qPow(nodes[i].y() - event->pos().y(), 2));
            if (distance < gripRadius) selectedNode = i;
        }
        if (nodes.count() > 2 && selectedNode > -1)
            update();
        else
            deleteNodeAction->setEnabled(false);
    }

    // Execute the menu synchronously
    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == insertNodeAction) insertNode(event->pos());
    if (selectedAction == deleteNodeAction) deleteNode();
    if (selectedAction == copyIDAction) copyID();
    if (selectedAction == copyFirstNodeAction) copyFirstNode();
    if (selectedAction == copyLastNodeAction) copyLastNode();
    if (selectedAction == pasteFirstNodeAction) pasteFirstNode();
    if (selectedAction == pasteLastNodeAction) pasteLastNode();
    if (selectedAction == deleteElementAction) deleteElement();

    if (selectedNode > -1)
    {
        selectedNode = -1;
        update();
    }
}

void PathElement::insertNode(QPointF I)
{
    qreal ABnorm, dotprod, distance;
    QPointF AI, AB, ABx;
    qreal r = (isWired ? wireW : normalW) / 2;

    for (int i = 0; i < nodes.count() - 1; ++i)
    {
        AI = I - nodes[i];
        AB = nodes[i + 1] - nodes[i];
        ABnorm = qSqrt(qPow(AB.x(), 2) + qPow(AB.y(), 2));
        dotprod = (AI.x() * AB.x() + AI.y() * AB.y()) / ABnorm;
        ABx = unitVector(nodes[i], nodes[i + 1], true);
        distance = (AI.x() * ABx.x() + AI.y() * ABx.y());
        if (0 <= dotprod && dotprod <= ABnorm && qFabs(distance) <= r)
        {
            AI = nodes[i] + AB * dotprod / ABnorm;
            nodes.insert(i + 1, AI);

            // Update center and border paths
            prepareGeometryChange();
            calcPaths();
            update();

            updateXML();
            return;
        }
    }
}

void PathElement::deleteNode()
{
    if (selectedNode > -1)
    {
        nodes.removeAt(selectedNode);

        // Update center and border paths
        prepareGeometryChange();
        calcPaths();
        update();

        updateXML();
    }
}

void PathElement::copyID()
{
    Item *item = static_cast<Item*>(modelIndex.internalPointer());
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(item->name);
}

void PathElement::copyFirstNode()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(nodes[0].x(), 'f', 2) + QString(",") + QString::number(nodes[0].y(), 'f', 2));
}

void PathElement::copyLastNode()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(nodes.last().x(), 'f', 2) + QString(",") + QString::number(nodes.last().y(), 'f', 2));
}

void PathElement::pasteFirstNode()
{
    QClipboard *clipboard = QApplication::clipboard();
    QStringList tokens = clipboard->text().split(",");
    if (tokens.count() == 2)
    {
        nodes[0] = QPointF(tokens[0].toDouble(), tokens[1].toDouble());

        // Update center and border paths
        prepareGeometryChange();
        calcPaths();
        update();

        updateXML();
    }
}

void PathElement::pasteLastNode()
{
    QClipboard *clipboard = QApplication::clipboard();
    QStringList tokens = clipboard->text().split(",");
    if (tokens.count() == 2)
    {
        nodes[nodes.count() - 1] = QPointF(tokens[0].toDouble(), tokens[1].toDouble());

        // Update center and border paths
        prepareGeometryChange();
        calcPaths();
        update();

        updateXML();
    }
}

void PathElement::deleteElement()
{
    qDebug() << "PathElement::deleteElement, Type: " << QString::number(type);
    
    Item *item = static_cast<Item*>(modelIndex.internalPointer());
    qDebug() << "PathElement::deleteElement, Name: " << item->name;
    
    if (!editable) { return; }

    // need to distinguish between EdgeAndLane, junction and connection
    switch (type)
    {
        case Edge:
            model->deleteEdgeAndLane(item); break;
        case EdgeNoShape:
            qDebug() << "PathElement: deleteElement - nothing to do to delete EdgeNoShape element";
            QMessageBox::information(NULL, "Sorry!", "Deletion of EdgeNoShape element not yet implemented!");
            break;
        case NormalLane:
            model->deleteEdgeAndLane(item); break;
        case IntLane:
            model->deleteEdgeAndLane(item); break;
        case PlainJunction:
            model->deleteJunction(item); break;
        case IntJunction:
            model->deleteJunction(item); break;
        case Connection:
            model->deleteConnection(item); break;
    }
        
}

void PathElement::updateXML()
{
    // Find the item referred by modelIndex and update the XML shape and length properties
    if (modelIndex.isValid())
    {
        Item *item = static_cast<Item*>(modelIndex.internalPointer());

        model->editAttribute(item->xmlNode, item->xmlSubNode, "shape", shapePoints());

        if (type == NormalLane || type == IntLane)
            model->editAttribute(item->xmlNode, item->xmlSubNode, "length", length());
    }
}

void PathElement::highlight()
{
    blink = 0;
    // Start timer
    timer.start(100, this);
}

void PathElement::highlightPoint(int node)
{
    blink = 0;
    blinkingNode = node;
    // Start timer
    timer.start(100, this);
}

void PathElement::timerEvent(QTimerEvent *)
{
    // Normally the blink parameter is -1; when the highlight methods are called it is changed to 0
    if (blink >= 0)
    {
        // Every time the timer emits a call, the element is selected or deselected
        if (blinkingNode < 0) selected = !selected;
        update();
        ++blink;

        // After 6 blinks, the timer is stopped
        if (blink == 6)
        {
            timer.stop();
            blink = -1;
            blinkingNode = -1;
        }
    }
}

Item* PathElement::getItem()
{
  return item;
}