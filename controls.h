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


#ifndef CONTROLS_H
#define CONTROLS_H

#include "model.h"

#include <QWidget>
#include <QCheckBox>

class Controls : public QWidget
{
    Q_OBJECT
public:
    // Constructor
    explicit Controls(QWidget *parent = 0);

    // Pointer to Model instance
    Model *model;

    // Resets all the checkboxes to the default status.
    void reset();

private slots:
    // Slots for each checkbox, which utilise the general private methods below
    void showLayer1(int state) const;
    void showLayer2(int state) const;
    void showLayer3(int state) const;
    void showLayer4(int state) const;
    void showLayer6(int state) const;
    void showLayer7(int state) const;

    void wireLayer1(int state) const;
    void wireLayer2(int state) const;
    void wireLayer3(int state) const;

    void arrowsLayer1(int state) const;
    void arrowsLayer2(int state) const;
    void arrowsLayer3(int state) const;
    void arrowsLayer7(int state) const;

    void editLayer1(int state) const;
    void editLayer2(int state) const;
    void editLayer3(int state) const;
    void editLayer4(int state) const;
    void editLayer6(int state) const;
    void editLayer7(int state) const;

private:
    // CheckBox handlers
    QCheckBox *view1, *view2, *view3, *view4, *view6, *view7;
    QCheckBox *wire1, *wire2, *wire3;
    QCheckBox *edit1, *edit2, *edit3, *edit4, *edit6, *edit7;
    QCheckBox *arrow1, *arrow2, *arrow3, *arrow7;

    // Changes a property of the Path/Point Elements within a layer
    void switchLayerState(Model::TreeBranch branch, bool subbranch, Model::ElementProperty prop, int state) const;
    void switchPointLayerState(Model::TreeBranch branch, Model::ElementProperty prop, int state) const;
};

#endif // CONTROLS_H
