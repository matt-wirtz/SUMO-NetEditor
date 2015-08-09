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


#include "networkview.h"
#include "pathelement.h"
#include "pointelement.h"
#include "item.h"

#include <QMouseEvent>
#include <QMessageBox>
#include <qmath.h>
#include <QDebug>

NetworkView::NetworkView() : QGraphicsView()
{
    // Switch off scroll bars and set properties
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setDragMode(ScrollHandDrag);
    setFocusPolicy(Qt::ClickFocus);

    // Initialise members
    zoom = 0;
    itemsLastClick = 0;
    currentIndex = 0;
}

void NetworkView::wheelEvent(QWheelEvent *event)
{
    // Zoom around the mouse pointer
    setTransformationAnchor(QGraphicsView::AnchorUnderMouse);

    // Increase or decrease zoom depending on wheel direction
    if (event->delta() > 0) zoom += 0.5; else zoom -= 0.5;

    qreal scale = qExp(zoom);

    // Adjust matrix accordingly; yScale is multiplied by -1 given
    // the Graphics View coordinates are in the opposite direction
    QMatrix matrix;
    matrix.scale(scale, -scale);
    setMatrix(matrix);
}

void NetworkView::zoomExtents()
{
    // Generate a zoom value so that both directions fit in the view
    if (width() / scene()->width() < height() / scene()->height())
        zoom = qLn(width() / scene()->width());
    else
        zoom = qLn(height() / scene()->height());

    qreal scale = qExp(zoom);

    // Adjust matrix accordingly; yScale is multiplied by -1 given
    // the Graphics View coordinates are in the opposite direction
    QMatrix matrix;
    matrix.scale(scale, -scale);
    setMatrix(matrix);
}

void NetworkView::mousePressEvent(QMouseEvent *event)
{
    // Store click position
    lastClick = event->pos();
    
    // Generate the list of graphics items under the mouse click
    itemList = items(lastClick);
    
    // Workaround: the function generateClickedIndexList accesses the selected bool value of PointElements and
    // PathElements. This bool value gets set in the slot Model::SelectionChanged which is triggered when the 
    // mousePressEvent is further processed.
    // Change: Use a slot in the NetworkView too that is called and creates the ClickedIndices list
    // Can the status bar still be updated with the number of selected items?
    // Process event
    QGraphicsView::mousePressEvent(event);

    generateClickedIndexList();
    itemsLastClick = clickedIndices.size();

    // Emit a message for the status bar with the mouse coordinates and the items under it
    QPointF currentPos = mapToScene(event->pos());
    QString message = tr("Position: ") + QString::number(currentPos.x(), 'f', 2) + ", " + QString::number(currentPos.y(), 'f', 2);
    if (itemsLastClick > 1) message += "         " + QString::number(itemsLastClick) + tr(" items @ last click  (toggle with spacebar)");
    emit updateStatusBar(message);

}

void NetworkView::mouseMoveEvent(QMouseEvent *event)
{
    // Emit a message for the status bar with the mouse coordinates
    QPointF currentPos = mapToScene(event->pos());
    QString message = tr("Position: ") + QString::number(currentPos.x(), 'f', 2) + ", " + QString::number(currentPos.y(), 'f', 2);
    if (itemsLastClick > 1) message += "         " + QString::number(itemsLastClick) + tr(" items @ last click  (toggle with spacebar)");
    emit updateStatusBar(message);

    // Process event
    QGraphicsView::mouseMoveEvent(event);
}

void NetworkView::keyPressEvent(QKeyEvent *event)
{
    // Page Up zooms in
    if (event->key() == Qt::Key_PageUp)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        zoom += 0.5;
        qreal scale = qExp(zoom);
        QMatrix matrix;
        matrix.scale(scale, -scale);
        setMatrix(matrix);
    }

    // Page Down zooms out
    if (event->key() == Qt::Key_PageDown)
    {
        setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
        zoom -= 0.5;
        qreal scale = qExp(zoom);
        QMatrix matrix;
        matrix.scale(scale, -scale);
        setMatrix(matrix);
    }

    // The space bar toggles the selection among all items at the point of the last click
    if (event->key() == Qt::Key_Space)
    {
        if ( !clickedIndices.isEmpty() ) {
            ++currentIndex;
            if (currentIndex >= clickedIndices.size()) currentIndex = 0;
            //selectionModel->select(clickedIndices[currentIndex], QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            
            if (clickedIndices[currentIndex]->hasPath) {
                selectionModel->select( clickedIndices[currentIndex]->graphicItem1->model->index(clickedIndices[currentIndex]), 
                    QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            } else if (clickedIndices[currentIndex]->hasPoint) {
                selectionModel->select( clickedIndices[currentIndex]->graphicItem2->model->index(clickedIndices[currentIndex]), 
                   QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
            }
        }
    }

    // The del key should delete the selected item
    // forwarding the deletion event to PathElement::deleteElement() 
    // or PointElement::deleteElement() in order to combine the event with the
    // deletion event comming from the context menu.
    // Delete events for junctions are forwarded to the PathElement class.
    if (event->key() == Qt::Key_Delete)
    {
        // only process event, if at least one object is selected
        if (clickedIndices.size() > 0)
        {
            QMessageBox::information(NULL, "NetworkView", "Received Delete Event");
            //Item *item = static_cast<Item*>(clickedIndices[currentIndex].internalPointer());
            Item *item = clickedIndices[currentIndex];
	    qDebug() << "NetworkView::keyPressEvent: Item Name: " << item->name;
    
            if (item->hasPath) {
                PathElement *pathit = item->graphicItem1;
                qDebug() << "NetworkView: delete pathElement, ElementType: " << QString::number(pathit->type);
                pathit->deleteElement();
            } else if (item->hasPoint) {
                PointElement *pointit = item->graphicItem2;
                qDebug() << "NetworkView: delete pointElement, ElementType " << QString::number(pointit->type);
        	pointit->deleteElement();
            } else {
                // what the heck is this of an element?
                qDebug() << "NetworkView: delete event received for an unknown type";
            }
        }
    }
}

void NetworkView::generateClickedIndexList()
{
    PathElement *pathit;
    PointElement *pointit;
    bool repeated;

    // Generate a list of pointers to the selected model items at the last click avoiding repetitions
    // Repetition can be caused by plain junctions with a path element and a point element
    clickedIndices.clear();
    for (int i = 0; i < itemList.count(); ++i)
    {
        pathit = dynamic_cast <PathElement*>(itemList[i]);
        if (pathit != NULL)
        {
            repeated = false;
            for (int j = 0; j < clickedIndices.size(); ++j)
                if (pathit->getItem() == clickedIndices[j])
                    repeated = true;
            if (!repeated)
            {
                clickedIndices.append(pathit->getItem());
                if (pathit->isSelected()) currentIndex = i;
            }
        }
        else
        {
            pointit = dynamic_cast <PointElement*>(itemList[i]);
            if (pointit != NULL)
            {
                repeated = false;
                for (int j = 0; j < clickedIndices.size(); ++j)
                    if (pointit->getItem() == clickedIndices[j])
                        repeated = true;
                if (!repeated)
                {
                    clickedIndices.append(pointit->getItem());
                    if (pointit->isSelected()) currentIndex = i;
                }
            }
        }
    }
    
    qDebug() << "NetworkView::generateClickedIndexList: itemList.count=" << QString::number(itemList.count()) 
      << ", currentIndex=" << QString::number(currentIndex);
}

void NetworkView::setSelectionModel(QItemSelectionModel *selectionModel)
{
    // Selection model setter
    this->selectionModel = selectionModel;
}
