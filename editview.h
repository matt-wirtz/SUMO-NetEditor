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


#ifndef EDITVIEW_H
#define EDITVIEW_H

#include "model.h"
#include "attredit.h"

#include <QWidget>
#include <QMainWindow>
#include <QItemSelection>
#include <QFormLayout>
#include <QLabel>

class EditView : public QWidget
{
    Q_OBJECT

public:
    // Constructor
    explicit EditView(QMainWindow *parent = 0);

    // Reference to model
    Model *model;

private slots:
    // Refreshes the widget with the attributes of the selected item
    void selectionChanged(QItemSelection on, QItemSelection off);

private:
    // Form layour handler
    QFormLayout *layout;

    // Individual captions handlers
    QList <QLabel*> field;
    QList <AttrEdit*> edit;

    // Lists of attributes for each element type, based on SUMO 0.20.0 xsd schema
    QStringList edgeAttr, laneAttr, juncAttr, connAttr, tlLgcAttr, reqAttr, phaseAttr;

    // Lists of bools to store whether the attributes are editable or not
    QList<bool> edgeEdit, laneEdit, juncEdit, connEdit, tlLgcEdit, reqEdit, phaseEdit;
};

#endif // EDITVIEW_H
