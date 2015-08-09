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


#include "controls.h"

#include <QLabel>
#include <QGridLayout>

Controls::Controls(QWidget *parent) : QWidget(parent)
{
    // Create labels and check boxes
    QLabel *icon1 = new QLabel; icon1->setPixmap(QPixmap(":/icons/edge1616.png"));
    QLabel *icon2 = new QLabel; icon2->setPixmap(QPixmap(":/icons/nmlLane1616.png"));
    QLabel *icon3 = new QLabel; icon3->setPixmap(QPixmap(":/icons/intLane1616.png"));
    QLabel *icon4 = new QLabel; icon4->setPixmap(QPixmap(":/icons/nmlJuncPoly1616.png"));
    QLabel *icon6 = new QLabel; icon6->setPixmap(QPixmap(":/icons/intJuncPoint1616.png"));
    QLabel *icon7 = new QLabel; icon7->setPixmap(QPixmap(":/icons/connection1616.png"));

    QLabel *labela = new QLabel(tr("Show"));
    QLabel *labelb = new QLabel(tr("Wire"));
    QLabel *labelc = new QLabel(tr("Arrow"));
    QLabel *labeld = new QLabel(tr("Edit"));
    QLabel *label1 = new QLabel(tr("Edges"));
    QLabel *label2 = new QLabel(tr("Normal Lanes"));
    QLabel *label3 = new QLabel(tr("Internal Lanes"));
    QLabel *label4 = new QLabel(tr("Plain Junctions"));
    QLabel *label6 = new QLabel(tr("Internal Junctions"));
    QLabel *label7 = new QLabel(tr("Connections"));

    view1 = new QCheckBox(); view1->setFocusPolicy(Qt::NoFocus);
    view2 = new QCheckBox(); view2->setFocusPolicy(Qt::NoFocus);
    view3 = new QCheckBox(); view3->setFocusPolicy(Qt::NoFocus);
    view4 = new QCheckBox(); view4->setFocusPolicy(Qt::NoFocus);
    view6 = new QCheckBox(); view6->setFocusPolicy(Qt::NoFocus);
    view7 = new QCheckBox(); view7->setFocusPolicy(Qt::NoFocus);

    wire1 = new QCheckBox(); wire1->setFocusPolicy(Qt::NoFocus);
    wire2 = new QCheckBox(); wire2->setFocusPolicy(Qt::NoFocus);
    wire3 = new QCheckBox(); wire3->setFocusPolicy(Qt::NoFocus);

    edit1 = new QCheckBox(); edit1->setFocusPolicy(Qt::NoFocus);
    edit2 = new QCheckBox(); edit2->setFocusPolicy(Qt::NoFocus);
    edit3 = new QCheckBox(); edit3->setFocusPolicy(Qt::NoFocus);
    edit4 = new QCheckBox(); edit4->setFocusPolicy(Qt::NoFocus);
    edit6 = new QCheckBox(); edit6->setFocusPolicy(Qt::NoFocus);
    edit7 = new QCheckBox(); edit7->setFocusPolicy(Qt::NoFocus);

    arrow1 = new QCheckBox(); arrow1->setFocusPolicy(Qt::NoFocus);
    arrow2 = new QCheckBox(); arrow2->setFocusPolicy(Qt::NoFocus);
    arrow3 = new QCheckBox(); arrow3->setFocusPolicy(Qt::NoFocus);
    arrow7 = new QCheckBox(); arrow7->setFocusPolicy(Qt::NoFocus);

    // Reset the checked status of each checkbox
    reset();

    // Arrange check boxes in a grid layout
    QGridLayout *layout = new QGridLayout;

    layout->addWidget(icon1, 1, 0, Qt::AlignHCenter);
    layout->addWidget(icon2, 2, 0, Qt::AlignHCenter);
    layout->addWidget(icon3, 3, 0, Qt::AlignHCenter);
    layout->addWidget(icon4, 4, 0, Qt::AlignHCenter);
    layout->addWidget(icon6, 6, 0, Qt::AlignHCenter);
    layout->addWidget(icon7, 7, 0, Qt::AlignHCenter);

    layout->addWidget(labela, 0, 2, Qt::AlignHCenter);
    layout->addWidget(labelb, 0, 3, Qt::AlignHCenter);
    layout->addWidget(labelc, 0, 4, Qt::AlignHCenter);
    layout->addWidget(labeld, 0, 5, Qt::AlignHCenter);
    layout->addWidget(label1, 1, 1, Qt::AlignLeft);
    layout->addWidget(label2, 2, 1, Qt::AlignLeft);
    layout->addWidget(label3, 3, 1, Qt::AlignLeft);
    layout->addWidget(label4, 4, 1, Qt::AlignLeft);
    layout->addWidget(label6, 6, 1, Qt::AlignLeft);
    layout->addWidget(label7, 7, 1, Qt::AlignLeft);

    layout->addWidget(view1, 1, 2, Qt::AlignHCenter);
    layout->addWidget(view2, 2, 2, Qt::AlignHCenter);
    layout->addWidget(view3, 3, 2, Qt::AlignHCenter);
    layout->addWidget(view4, 4, 2, Qt::AlignHCenter);
    layout->addWidget(view6, 6, 2, Qt::AlignHCenter);
    layout->addWidget(view7, 7, 2, Qt::AlignHCenter);

    layout->addWidget(wire1, 1, 3, Qt::AlignHCenter);
    layout->addWidget(wire2, 2, 3, Qt::AlignHCenter);
    layout->addWidget(wire3, 3, 3, Qt::AlignHCenter);

    layout->addWidget(arrow1, 1, 4, Qt::AlignHCenter);
    layout->addWidget(arrow2, 2, 4, Qt::AlignHCenter);
    layout->addWidget(arrow3, 3, 4, Qt::AlignHCenter);
    layout->addWidget(arrow7, 7, 4, Qt::AlignHCenter);

    layout->addWidget(edit1, 1, 5, Qt::AlignHCenter);
    layout->addWidget(edit2, 2, 5, Qt::AlignHCenter);
    layout->addWidget(edit3, 3, 5, Qt::AlignHCenter);
    layout->addWidget(edit4, 4, 5, Qt::AlignHCenter);
    layout->addWidget(edit6, 6, 5, Qt::AlignHCenter);
    layout->addWidget(edit7, 7, 5, Qt::AlignHCenter);

    // Connect signals and slots
    connect(view1, SIGNAL(stateChanged(int)), this, SLOT(showLayer1(int)));
    connect(view2, SIGNAL(stateChanged(int)), this, SLOT(showLayer2(int)));
    connect(view3, SIGNAL(stateChanged(int)), this, SLOT(showLayer3(int)));
    connect(view4, SIGNAL(stateChanged(int)), this, SLOT(showLayer4(int)));
    connect(view6, SIGNAL(stateChanged(int)), this, SLOT(showLayer6(int)));
    connect(view7, SIGNAL(stateChanged(int)), this, SLOT(showLayer7(int)));

    connect(wire1, SIGNAL(stateChanged(int)), this, SLOT(wireLayer1(int)));
    connect(wire2, SIGNAL(stateChanged(int)), this, SLOT(wireLayer2(int)));
    connect(wire3, SIGNAL(stateChanged(int)), this, SLOT(wireLayer3(int)));

    connect(edit1, SIGNAL(stateChanged(int)), this, SLOT(editLayer1(int)));
    connect(edit2, SIGNAL(stateChanged(int)), this, SLOT(editLayer2(int)));
    connect(edit3, SIGNAL(stateChanged(int)), this, SLOT(editLayer3(int)));
    connect(edit4, SIGNAL(stateChanged(int)), this, SLOT(editLayer4(int)));
    connect(edit6, SIGNAL(stateChanged(int)), this, SLOT(editLayer6(int)));
    connect(edit7, SIGNAL(stateChanged(int)), this, SLOT(editLayer7(int)));

    connect(arrow1, SIGNAL(stateChanged(int)), this, SLOT(arrowsLayer1(int)));
    connect(arrow2, SIGNAL(stateChanged(int)), this, SLOT(arrowsLayer2(int)));
    connect(arrow3, SIGNAL(stateChanged(int)), this, SLOT(arrowsLayer3(int)));
    connect(arrow7, SIGNAL(stateChanged(int)), this, SLOT(arrowsLayer7(int)));

    // Set layout in widget
    setLayout(layout);
    setSizePolicy(QSizePolicy::Preferred , QSizePolicy::Fixed );
}

// Call switchLayerState or switchPointLayerState with the right parameters, according to the checkbox clicked
void Controls::showLayer1(int state) const { switchLayerState(Model::NEdgesBranch, false, Model::ViewElement, state); switchLayerState(Model::IEdgesBranch, false, Model::ViewElement, state); }
void Controls::showLayer2(int state) const { switchLayerState(Model::NEdgesBranch, true, Model::ViewElement, state); }
void Controls::showLayer3(int state) const { switchLayerState(Model::IEdgesBranch, true, Model::ViewElement, state); }

void Controls::showLayer4(int state) const 
{ 
    switchLayerState(Model::PJuncsBranch, false, Model::ViewElement, state); 
    switchPointLayerState(Model::PJuncsBranch, Model::ViewElement, state); 
}

void Controls::showLayer6(int state) const { switchPointLayerState(Model::IJuncsBranch, Model::ViewElement, state); }
void Controls::showLayer7(int state) const { switchLayerState(Model::ConnsBranch, false, Model::ViewElement, state); switchPointLayerState(Model::ConnsBranch, Model::ViewElement, state); }

void Controls::wireLayer1(int state) const { switchLayerState(Model::NEdgesBranch, false, Model::WireElement, state); switchLayerState(Model::IEdgesBranch, false, Model::WireElement, state); }
void Controls::wireLayer2(int state) const { switchLayerState(Model::NEdgesBranch, true, Model::WireElement, state); }
void Controls::wireLayer3(int state) const { switchLayerState(Model::IEdgesBranch, true, Model::WireElement, state); }

void Controls::editLayer1(int state) const { switchLayerState(Model::NEdgesBranch, false, Model::EditElement, state); switchLayerState(Model::IEdgesBranch, false, Model::EditElement, state); }
void Controls::editLayer2(int state) const { switchLayerState(Model::NEdgesBranch, true, Model::EditElement, state); }
void Controls::editLayer3(int state) const { switchLayerState(Model::IEdgesBranch, true, Model::EditElement, state); }

void Controls::editLayer4(int state) const 
{   switchLayerState(Model::PJuncsBranch, false, Model::EditElement,state);
    switchPointLayerState(Model::PJuncsBranch, Model::EditElement, state);
}

void Controls::editLayer6(int state) const { switchPointLayerState(Model::IJuncsBranch, Model::EditElement, state); }

void Controls::editLayer7(int state) const 
{
    switchLayerState(Model::ConnsBranch, false, Model::EditElement, state);
    switchPointLayerState(Model::ConnsBranch, Model::EditElement, state); 
}

void Controls::arrowsLayer1(int state) const { switchLayerState(Model::NEdgesBranch, false, Model::ArrowElement, state); switchLayerState(Model::IEdgesBranch, false, Model::ArrowElement, state); }
void Controls::arrowsLayer2(int state) const { switchLayerState(Model::NEdgesBranch, true, Model::ArrowElement, state); }
void Controls::arrowsLayer3(int state) const { switchLayerState(Model::IEdgesBranch, true, Model::ArrowElement, state); }
void Controls::arrowsLayer7(int state) const { switchLayerState(Model::ConnsBranch, false, Model::ArrowElement, state); }


void Controls::switchLayerState(Model::TreeBranch branch, bool subbranch, Model::ElementProperty prop, int state) const
{
    if (model)
    {
        // Convert the check box state (three possible values) to boolean
        bool bstate = (state == Qt::Checked ? true : false);

        // Redirect the request to the model
        model->switchLayerState(branch, subbranch, prop, bstate);
    }
}

void Controls::switchPointLayerState(Model::TreeBranch branch, Model::ElementProperty prop, int state) const
{
    if (model)
    {
        // Convert the check box state (three possible values) to boolean
        bool bstate = (state == Qt::Checked ? true : false);

        // Redirect the request to the model
        model->switchPointLayerState(branch, prop, bstate);
    }
}

void Controls::reset()
{
    // Resets all the check boxes to the default values
    view1->setChecked(true);
    view2->setChecked(true);
    view3->setChecked(true);
    view4->setChecked(true);
    view6->setChecked(true);
    view7->setChecked(true);

    wire1->setChecked(false);
    wire2->setChecked(false);
    wire3->setChecked(false);

    edit1->setChecked(false);
    edit2->setChecked(false);
    edit3->setChecked(false);
    edit4->setChecked(false);
    edit6->setChecked(false);
    edit7->setChecked(false);

    arrow1->setChecked(false);
    arrow2->setChecked(false);
    arrow3->setChecked(false);
    arrow7->setChecked(false);
}
