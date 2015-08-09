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


#include "pointelement.h"
#include "item.h"

#include <QPen>
#include <QBrush>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QDebug>

PointElement::PointElement(ElementType type, qreal x, qreal y, Model *model, Item *item, QItemSelectionModel *selectionModel) : QGraphicsEllipseItem()
{
    // Initialise members
    this->type = type;
    this->x = x;
    this->y = y;
    this->model = model;
    this->item = item;
    this->selectionModel = selectionModel;

    // Set element properties
    switch (type)
    {
        case PlainJunction:
            r = 0;   g = 100; b = 0;   radius = 0.3;  w = 0.3;  z = 6; break;
        case IntJunction:
            r = 128; g = 0;   b = 128; radius = 0.3;  w = 0.3;  z = 7; break;
        case Connection:
            r = 255; g = 255; b = 0;   radius = 0.2;  w = 0.15; z = 4; break;
    }
    setRect(x - radius, y - radius, 2 * radius, 2 * radius);
    setPen(QPen(QColor(r, g, b), w));
    setZValue(z);

    // Initialise more members
    selected = false;
    editable = false;
    moving = false;

    // Set arrow cursor
    setCursor(QCursor(Qt::ArrowCursor));

    update();
}

void PointElement::select()
{
    // Set selected as true, set colour as red, bring element to front and redraw
    selected = true;
    setPen(QPen(QColor(255, 0, 0), w));
    setZValue(10);
    update();
}

void PointElement::deselect()
{
    // Set selected as false, set normal colour, put element back into its normal position and redraw
    selected = false;
    setPen(QPen(QColor(r, g, b), w));
    setZValue(z);
    update();
}

bool PointElement::isSelected() const
{
    return selected;
}

void PointElement::switchState(Model::ElementProperty prop, bool state)
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
    default: break;
    }
}

void PointElement::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    selected = true;

    // Prepare the element to be moved
    if (editable)
    {
        moving = true;
        lastPos = event->pos();
        update();
    }

    // Select the element in the tree and trigger all the selection processes
    if (selectionModel)
        selectionModel->select(modelIndex, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
}

void PointElement::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    // Move the element if previously clicked on it (and is in edit state)
    if (moving)
    {
        x += event->pos().x() - lastPos.x();
        y += event->pos().y() - lastPos.y();
        lastPos = event->pos();
        prepareGeometryChange();
        setRect(x - radius, y - radius, 2 * radius, 2 * radius);
        update();
    }
}

void PointElement::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
    // Update the XML domDocument if the element was moved
    if (moving)
    {
        moving = false;
        updateXML();
        update();
    }
}

void PointElement::updateXML()
{
    // Find the item referred by modelIndex and update the XML x and y properties
    if (modelIndex.isValid())
    {
        Item *item = static_cast<Item*>(modelIndex.internalPointer());
        model->editAttribute(item->xmlNode, item->xmlSubNode, "x",  QString::number(x, 'f', 2));
        model->editAttribute(item->xmlNode, item->xmlSubNode, "y",  QString::number(y, 'f', 2));
    }
}

void PointElement::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    // Create context menu
    QMenu menu;
    QAction *copyCoordsAction = menu.addAction(QObject::tr("Copy coordinates"));
    QAction *pasteCoordsAction = menu.addAction(QObject::tr("Paste coordinates"));
    menu.addSeparator();
    QAction *deleteElementAction = menu.addAction(QObject::tr("Delete element"));

    if (!editable)
    {
        pasteCoordsAction->setEnabled(false);
        deleteElementAction->setEnabled(false);
    }

    // Execute the menu synchronously
    QAction *selectedAction = menu.exec(event->screenPos());

    if (selectedAction == copyCoordsAction) copyCoords();
    if (selectedAction == pasteCoordsAction) pasteCoords();
    if (selectedAction == deleteElementAction) deleteElement();
}

void PointElement::copyCoords()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(QString::number(x, 'f', 2) + QString(",") + QString::number(y, 'f', 2));
}

void PointElement::pasteCoords()
{
    QClipboard *clipboard = QApplication::clipboard();
    QStringList tokens = clipboard->text().split(",");
    if (tokens.count() == 2)
    {
        x = tokens[0].toDouble();
        y = tokens[1].toDouble();

        // Update center and border paths
        prepareGeometryChange();
        setRect(x - radius, y - radius, 2 * radius, 2 * radius);
        update();

        updateXML();
    }
}

void PointElement::deleteElement()
{
    qDebug() << "PointElement::deleteElement: name=" << item->name;
    
    if (!editable) { return; }
    
    // only delete events for connections need to be handled
    // junctions are already handled by PathElement::deleteElement()
    if (type == PointElement::Connection) {
       model->deleteConnection(item);
    } else {
        qDebug() << "PointElement::deleteElement: unknown type";
    }
}

void PointElement::highlight()
{
    blink = 0;

    // Start timer
    timer.start(100, this);
}

void PointElement::timerEvent(QTimerEvent *)
{
    // Normally the blink parameter is -1; when the highlight methods are called it is changed to 0
    if (blink >= 0)
    {
        // Every time the timer emits a call, the element is selected or deselected
        if (selected) deselect(); else select();
        update();
        ++blink;

        // After 6 blinks, the timer is stopped
        if (blink == 6)
        {
            timer.stop();
            blink = -1;
        }
    }
}


Item* PointElement::getItem()
{
  return item;
}
