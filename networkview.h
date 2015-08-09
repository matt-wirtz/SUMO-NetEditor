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


#ifndef NETWORKVIEW_H
#define NETWORKVIEW_H

#include "item.h"
#include <QGraphicsView>
#include <QPoint>
#include <QItemSelectionModel>

class NetworkView : public QGraphicsView
{
    Q_OBJECT
public:
    // Constructor
    NetworkView();

    // Adjusts the zoom (matrix) so that the whole network is visualised
    void zoomExtents();

    // Sets item selection model
    void setSelectionModel(QItemSelectionModel *selectionModel);

signals:
    // Generates a message with the current mouse coordinates and number of items in last click
    void updateStatusBar(QString message);

protected:
    // Mouse and keyboard events
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    // Zoom
    qreal zoom;

    // Pointer to the item selection model
    QItemSelectionModel *selectionModel;

    // Point of last click and items underneath it
    QPoint lastClick;

    // List of pointers to all the graphic items in the last click
    QList<QGraphicsItem *> itemList;

    // Vector of all the model indexes (of the tree model) to the graphic items in the last click
    //QVector <QModelIndex> clickedIndices;
    QList<Item*> clickedIndices;

    // Generates 'clickIndices' from 'itemList'
    void generateClickedIndexList();

    // Items in the last click and current index of them
    int itemsLastClick, currentIndex;
};

#endif // NETWORKVIEW_H
