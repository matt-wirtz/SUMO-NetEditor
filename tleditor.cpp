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


#include "tleditor.h"
#include "model.h"
#include "item.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <qmath.h>
#include <QMenu>
#include <QContextMenuEvent>
#include <QMessageBox>

TLEditor::TLEditor(QWidget *parent, Model *model, Item *item) : QDialog(parent)
{
    // Initialise a few members
    this->model = model;
    this->item = item;
    oldrow = -1;

    // Get tlLogic Program ID
    int xmlNo = item->xmlNode;
    QString programid = model->getXMLelement(xmlNo).attribute("programID");

    // Set dialog properties
    setModal(true);
    setWindowTitle(tr("tlLogic Editor: ") + item->name + "  programID: " + programid);
    QIcon tlLogicIcon;
    tlLogicIcon = QPixmap(":/icons/tllogic1616.png");
    setWindowIcon(tlLogicIcon);

    // Create the junction diagram scene and view
    jctView = new QGraphicsView;
    jctScene = new QGraphicsScene;
    jctScene->setBackgroundBrush(QBrush(QColor(192, 192, 192)));
    cycleTimeLabel = new QLabel(tr("cycle time = "));
    cycleTimeLabel->setAlignment(Qt::AlignRight);

    // Create the junction diagram and the table
    createDiagram();
    createTable();

    // Set the diagram scene
    jctView->setScene(jctScene);
    QMatrix matrix;
    matrix.scale(1, -1);
    jctView->setMatrix(matrix);
    jctView->fitInView(jctScene->sceneRect(), Qt::KeepAspectRatio);

    // Create the buttons and place them in a horizontal layout
    QPushButton *apply = new QPushButton(tr("Apply"), this);
    QPushButton *cancel = new QPushButton(tr("Cancel"), this);
    apply->setFocusPolicy(Qt::NoFocus);
    cancel->setFocusPolicy(Qt::NoFocus);
    QHBoxLayout *blayout = new QHBoxLayout;
    blayout->addWidget(apply);
    blayout->addWidget(cancel);

    // Create a vertical layout and place the table, labels and button layout;
    // fit everything in a widget for the splitter
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addWidget(new QLabel("phases:"));
    vlayout->addWidget(phaseTable);
    vlayout->addWidget(cycleTimeLabel);
    vlayout->addLayout(blayout);
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setLayout(vlayout);

    // Create a splitter with the diagram view on the left and the table on the right
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(jctView);
    splitter->addWidget(rightPanel);
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addWidget(splitter);
    setLayout(mainlayout);

    // Connect signals and slots
    connect(phaseTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(cellChanged(int,int,int, int)));
    connect(phaseTable, SIGNAL(cellChanged(int,int)), this, SLOT(updateCycleTime(int,int)));
    connect(phaseTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(cellDoubleClicked(int,int)));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(apply, SIGNAL(clicked()), this, SLOT(applyChanges()));
    connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));
}

void TLEditor::resizeEvent(QResizeEvent *)
{
    // Keep the whole junction diagram in the view when it is resized
    jctView->fitInView(jctScene->sceneRect(), Qt::KeepAspectRatio);
}

void TLEditor::splitterMoved()
{
    // Keep the whole junction diagram in the view when it is resized
    jctView->fitInView(jctScene->sceneRect(), Qt::KeepAspectRatio);
}

void TLEditor::createDiagram()
{
    // Find the internal lanes attribute of the junction and parse it.
    // The junction has the same name as the tlLogic (item)
    Item *jctItem = model->getJunction(item->name);
    int xmlNo = jctItem->xmlNode;
    intLanes = model->getXMLelement(xmlNo).attribute("intLanes");
    QStringList tokens = intLanes.simplified().split(" ");
    intLanes = tr("Internal lanes: \n");

    // Draw the junction polygon
    jctScene->addPath(jctItem->graphicItem1->centerLine(), QPen(QColor(0, 100, 0), 0.2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin), QColor(0, 100, 0, 100));

    // Draw each individual lane and arrow in the scene, and keep a pointer to the lane in
    // laneDiagram so that the colour can be changed when the cell is changed
    QPainterPath path, arrow;
    qreal x0, y0, x1, y1, xu, yu, norm;
    for (int i = 0; i < tokens.size(); ++i)
    {
        // Add lane id to the intLanes label (deprecated)
        intLanes = intLanes + tokens[i] + "\n";

        // Add lane to the diagram and store the pointer in laneDiagram
        path = model->getInternalLane(tokens[i])->graphicItem1->centerLine();
        laneDiagram.append(jctScene->addPath(path, QPen(QColor(0, 0, 139), 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin), Qt::NoBrush));

        // Create arrow
        x0 = path.elementAt(path.elementCount() - 1).x;
        y0 = path.elementAt(path.elementCount() - 1).y;
        x1 = path.elementAt(path.elementCount() - 2).x;
        y1 = path.elementAt(path.elementCount() - 2).y;
        norm = qSqrt(qPow(x0 - x1, 2) + qPow(y0 - y1, 2));
        if (norm > 0)
        {
            xu = (x1 - x0) / norm;
            yu = (y1 - y0) / norm;
        }
        else
            xu = yu = 0;
        arrow = QPainterPath();
        arrow.moveTo(x0, y0);
        arrow.lineTo(x0 + xu - 0.4 * yu, y0 + yu + 0.4 * xu);
        arrow.lineTo(x0 + xu + 0.4 * yu, y0 + yu - 0.4 * xu);
        arrow.closeSubpath();
        jctScene->addPath(arrow, QPen(Qt::black, 0), QBrush(Qt::black));
    }
    // Store the number of lanes in the junction
    lanes = tokens.length();
}

void TLEditor::createTable()
{
    // Create tables
    phases = item->childCount();
    phaseTable = new QTableWidget(phases, lanes + 1);

    // Switch off horizontal scroll bar
    phaseTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // phaseTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set the table headers and row and column dimensions
    for (int i = 0; i < lanes; ++i)
    {
        phaseTable->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(i), 0));
        phaseTable->setColumnWidth(i, 18);
    }
    phaseTable->setHorizontalHeaderItem(lanes, new QTableWidgetItem("dur", 0));
    phaseTable->setColumnWidth(lanes, 18 * 2);

    for (int i = 0; i < phases; ++i)
    {
        phaseTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i), 0));
        phaseTable->setRowHeight(i, 18);
    }

    // Get the XML element from the model and fill in the table
    int xmlNo = item->xmlNode;
    QDomElement element = model->getXMLelement(xmlNo);
    QString state, duration;

    for (int i = 0; i < phases; ++i)
    {
        int n = 0;
        // Sweep all the 'phases' until the order matches the row number, and get the state and duration attributes
        for (int k = 0; k < element.childNodes().length(); ++k)
            if (element.childNodes().at(k).nodeName() == "phase" && i == n)
            {
                state = element.childNodes().at(k).toElement().attribute("state");
                duration = element.childNodes().at(k).toElement().attribute("duration");
                break;
            }
            else ++n;
        // Create each cell and fill in with a character of the state attribute
        for (int j = 0; j < lanes; ++j)
        {
            QTableWidgetItem *cell = new QTableWidgetItem(state.mid(j, 1), 0);
            cell->setFlags(Qt::ItemIsEnabled);
            cell->setTextAlignment(Qt::AlignHCenter + Qt::AlignVCenter);
            phaseTable->setItem(i, j, cell);
        }
        // Create one cell at the end of the row with the duration attribute
        QTableWidgetItem *cell = new QTableWidgetItem(duration, 0);
        //cell->setFlags(Qt::ItemIsEnabled);
        cell->setTextAlignment(Qt::AlignHCenter + Qt::AlignVCenter);
        phaseTable->setItem(i, lanes, cell);
    }
    // Update cycle time
    updateCycleTime(0, lanes);
}

void TLEditor::cellChanged(int row, int col, int dummyoldrow, int dummyoldcol)
{
    Q_UNUSED(col)
    Q_UNUSED(dummyoldrow)
    Q_UNUSED(dummyoldcol)

    // Unpaint the current row
    if (oldrow > -1)
        for (int i = 0; i < lanes; ++i)
            phaseTable->item(oldrow, i)->setBackground(Qt::NoBrush);

    // Paint each cell of the new selected row based on the cell value
    QColor colour;
    for (int i = 0; i < lanes; ++i)
    {
        // Determine colour based on value
        if (phaseTable->item(row, i)->text() == "r") colour = Qt::red;
        else if (phaseTable->item(row, i)->text() == "y") colour = Qt::yellow;
        else if (phaseTable->item(row, i)->text() == "G") colour = Qt::green;
        else if (phaseTable->item(row, i)->text() == "g") colour = Qt::darkGreen;

        // Paint the cell
        phaseTable->item(row, i)->setBackground(colour);

        // Paint the lane in the diagram
        laneDiagram[i]->setPen(QPen(colour, 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
    }
    oldrow = row;
}

void TLEditor::updateCycleTime(int row, int col)
{
    Q_UNUSED(row)

    // If the cell changed is in the last column, update the cycle time
    if (col == lanes)
    {
        // Add all the cell in the last column and update the cycle time label
        qreal cycleTime = 0.0;
        for (int i = 0; i < phases; ++i)
            cycleTime += phaseTable->item(i, lanes)->text().toDouble();
        cycleTimeLabel->setText(tr("cycle time = %1s").arg(QString::number(cycleTime, 'f', 1)));
    }
}

void TLEditor::cellDoubleClicked(int row, int col)
{
    // Rotate between the four possible states (r -> y -> g -> G -> r)
    if (phaseTable->item(row, col)->text() == "r") phaseTable->item(row, col)->setText("y");
    else if (phaseTable->item(row, col)->text() == "y") phaseTable->item(row, col)->setText("g");
    else if (phaseTable->item(row, col)->text() == "g") phaseTable->item(row, col)->setText("G");
    else if (phaseTable->item(row, col)->text() == "G") phaseTable->item(row, col)->setText("r");

    // Repaint the tables and junction diagram
    cellChanged(row, col, 0, 0);
}

void TLEditor::applyChanges()
{
    int xmlNo = item->xmlNode;
    QString state;

    for (int i = 0; i < phases; ++i)
    {
        // Concatenate each cell of the row into a string
        state = "";
        for (int j = 0; j < lanes; ++j)
            state = state + phaseTable->item(i, j)->text();

        // Update the XML element
        model->editAttribute(xmlNo, i, "state", state);
        model->editAttribute(xmlNo, i, "duration", phaseTable->item(i, lanes)->text());
    }
    // Close dialog box
    close();
}
