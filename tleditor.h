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


#ifndef TLEDITOR_H
#define TLEDITOR_H

#include "model.h"

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QLabel>

class TLEditor : public QDialog
{
    Q_OBJECT
public:
    // Constructor
    explicit TLEditor(QWidget *parent = 0, Model *model = 0, Item *item = 0);

protected:
    // Resizes the diagram in the view when the dialog size is changed
    void resizeEvent(QResizeEvent *);

private slots:
    // Resizes the diagram in the view when the splitter is moved
    void splitterMoved();

    // Sets the column and row colours in the table and the updates the diagram when the cell is changed
    void cellChanged(int row, int col, int dummyoldrow, int dummyoldcol);

    // Rotates between the valid values in the cell
    void cellDoubleClicked(int row, int col);

    // Updates the cycle time label if any phase duration is changed
    void updateCycleTime(int row, int col);

    // Updates the model with the changes when the "Apply" button is clicked
    void applyChanges();

private:
    // Creates the junction diagram; called by the constructor
    void createDiagram();

    // Creates the table; called by the constructor
    void createTable();

    // Diagram, table and cycle time label
    QGraphicsView *jctView;
    QGraphicsScene *jctScene;
    QTableWidget *phaseTable;
    QLabel *cycleTimeLabel;

    // Pointers to the model and the junction being edited
    Model *model;
    Item *item;

    // Number of lanes and phases in the junction
    int lanes, phases;

    // Current row
    int oldrow;

    // Pointers to the diagram lanes, used when when changing their colours
    QList <QGraphicsPathItem*> laneDiagram;

    // String with lane ids (deprecated)
    QString intLanes;
};

#endif // TLEDITOR_H
