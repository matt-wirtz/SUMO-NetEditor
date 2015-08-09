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


#include "jcteditor.h"
#include "model.h"
#include "item.h"

#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <QPushButton>
#include <qmath.h>
#include <QScrollBar>

JctEditor::JctEditor(QWidget *parent, Model *model, Item *item) : QDialog(parent)
{
    // Initialise a few members
    this->model = model;
    this->item = item;
    oldrow = -1;
    oldcol = -1;

    // Set dialog properties
    setModal(true);
    setWindowTitle(tr("Junction Editor: ") + item->name);
    QIcon nmlJuncIcon;
    nmlJuncIcon = QPixmap(":/icons/nmlJunc1616.png");
    setWindowIcon(nmlJuncIcon);

    // Create the junction diagram scene and view
    jctView = new QGraphicsView;
    jctScene = new QGraphicsScene;
    jctScene->setBackgroundBrush(QBrush(QColor(192, 192, 192)));

    // Create the junction diagram and the tables
    createDiagram();
    createTables();

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

    // Create a vertical layout and place the two tables and the button layout
    // plus a couple of labels; fit everything in a widget for the splitter
    QVBoxLayout *vlayout = new QVBoxLayout;
    //QLabel *text = new QLabel(intLanes);
    //text->setWordWrap(true);
    //vlayout->addWidget(text);
    vlayout->addWidget(new QLabel("response:"));
    vlayout->addWidget(responseTable);
    vlayout->addWidget(new QLabel("foes:"));
    vlayout->addWidget(foesTable);
    vlayout->addLayout(blayout);
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setLayout(vlayout);

    // Create a splitter with the diagram view on the left and the tables on the right
    QSplitter *splitter = new QSplitter(Qt::Horizontal, this);
    splitter->addWidget(jctView);
    splitter->addWidget(rightPanel);
    QVBoxLayout *mainlayout = new QVBoxLayout(this);
    mainlayout->addWidget(splitter);
    setLayout(mainlayout);

    // Connect signals and slots
    connect(responseTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(cellChanged(int,int,int, int)));
    connect(foesTable, SIGNAL(currentCellChanged(int,int,int,int)), this, SLOT(cellChanged(int,int,int, int)));
    connect(responseTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(cellDoubleClicked(int,int)));
    connect(foesTable, SIGNAL(cellDoubleClicked(int,int)), this, SLOT(cellDoubleClicked(int,int)));
    connect(cancel, SIGNAL(clicked()), this, SLOT(close()));
    connect(apply, SIGNAL(clicked()), this, SLOT(applyChanges()));
    connect(splitter, SIGNAL(splitterMoved(int,int)), this, SLOT(splitterMoved()));
}

void JctEditor::resizeEvent(QResizeEvent *)
{
    // Keep the whole junction diagram in the view when it is resized
    jctView->fitInView(jctScene->sceneRect(), Qt::KeepAspectRatio);
}

void JctEditor::splitterMoved()
{
    // Keep the whole junction diagram in the view when it is resized
    jctView->fitInView(jctScene->sceneRect(), Qt::KeepAspectRatio);
}


void JctEditor::createDiagram()
{
    // Find the internal lanes attribute in the junction and parse it
    int xmlNo = item->xmlNode;
    QDomElement element = model->getXMLelement(xmlNo);
    intLanes = element.attribute("intLanes");
    QStringList tokens = intLanes.simplified().split(" ");
    intLanes = tr("Internal lanes: \n");

    // Draw the junction polygon
    jctScene->addPath(item->graphicItem1->centerLine(), QPen(QColor(0, 100, 0), 0.2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin), QColor(0, 100, 0, 100));

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

void JctEditor::createTables()
{
    // Create tables
    responseTable = new QTableWidget(lanes, lanes);
    foesTable = new QTableWidget(lanes, lanes);

    // Switch off horizontal scroll bars and synchronise vertical bars on both tables
    responseTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    foesTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    QScrollBar *respVBar = responseTable->verticalScrollBar();
    QScrollBar *foesVBar = foesTable->verticalScrollBar();
    connect(respVBar, SIGNAL(valueChanged(int)), foesVBar, SLOT(setValue(int)));
    connect(foesVBar, SIGNAL(valueChanged(int)), respVBar, SLOT(setValue(int)));
    //responseTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //foesTable->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    // Set the table headers and row and column dimensions
    for (int i = 0; i < lanes; ++i)
    {
        responseTable->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(lanes - i - 1), 0));
        responseTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i), 0));
        foesTable->setHorizontalHeaderItem(i, new QTableWidgetItem(QString::number(lanes - i - 1), 0));
        foesTable->setVerticalHeaderItem(i, new QTableWidgetItem(QString::number(i), 0));
        responseTable->setColumnWidth(i, 18);
        responseTable->setRowHeight(i, 18);
        foesTable->setColumnWidth(i, 18);
        foesTable->setRowHeight(i, 18);
    }

    // Get the XML element from the model and fill in the tables
    int xmlNo = item->xmlNode;
    QDomElement element = model->getXMLelement(xmlNo);
    QString response, foes;

    for (int i = 0; i < lanes; ++i)
    {
        // Sweep all the 'requests' until the index matches the row number, and get the response and foes attributes
        for (int k = 0; k < element.childNodes().length(); ++k)
            if (element.childNodes().at(k).nodeName() == "request" && element.childNodes().at(k).toElement().attribute("index") == QString::number(i))
            {
                response = element.childNodes().at(k).toElement().attribute("response");
                foes = element.childNodes().at(k).toElement().attribute("foes");
            }
        // Create each cell and fill in with a character of the response/foes attribute
        for (int j = 0; j < lanes; ++j)
        {
            QTableWidgetItem *cell = new QTableWidgetItem(response.mid(j, 1), 0);
            cell->setFlags(Qt::ItemIsEnabled);
            cell->setTextAlignment(Qt::AlignHCenter + Qt::AlignVCenter);
            responseTable->setItem(i, j, cell);
            cell = new QTableWidgetItem(foes.mid(j, 1), 0);
            cell->setFlags(Qt::ItemIsEnabled);
            cell->setTextAlignment(Qt::AlignHCenter + Qt::AlignVCenter);
            foesTable->setItem(i, j, cell);
        }
    }
}

void JctEditor::cellChanged(int row, int col, int dummyoldrow, int dummyoldcol)
{
    Q_UNUSED(dummyoldrow)
    Q_UNUSED(dummyoldcol)

    // Determine row and column colours
    QColor rowColour, colColour;
    if (foesTable->item(row, col)->text() == "1" && responseTable->item(row, col)->text() == "1")
    {
        rowColour = Qt::red; colColour = Qt::green;
    }
    else if (foesTable->item(row, col)->text() == "1" && responseTable->item(row, col)->text() == "0")
    {
        rowColour = Qt::green; colColour = Qt::red;
    }
    else
    {
        rowColour = QColor(204, 204, 0); colColour = QColor(204, 153, 0);
    }

    // Unpaint the current rows and columns in both tables
    if (oldcol > -1)
        for (int i = 0; i < lanes; ++i)
        {
            responseTable->item(i, oldcol)->setBackground(Qt::NoBrush);
            foesTable->item(i, oldcol)->setBackground(Qt::NoBrush);
        }
    if (oldrow > -1)
        for (int i = 0; i < lanes; ++i)
        {
            responseTable->item(oldrow, i)->setBackground(Qt::NoBrush);
            foesTable->item(oldrow, i)->setBackground(Qt::NoBrush);
        }

    // Paint the new selected row and columns in both tables
    for (int i = 0; i < lanes; ++i)
    {
        responseTable->item(i, col)->setBackground(colColour);
        foesTable->item(i, col)->setBackground(colColour);
    }
    for (int i = 0; i < lanes; ++i)
    {
        responseTable->item(row, i)->setBackground(rowColour);
        foesTable->item(row, i)->setBackground(rowColour);
    }

    // Paint the deselected lanes in blue
    if (oldrow > -1) laneDiagram[oldrow]->setPen(QPen(QColor(0, 0, 139), 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
    if (oldcol > -1) laneDiagram[lanes - oldcol - 1]->setPen(QPen(QColor(0, 0, 139), 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
    // Paint the selected lanes in the appropriate colours
    laneDiagram[lanes - col - 1]->setPen(QPen(colColour, 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));
    laneDiagram[row]->setPen(QPen(rowColour, 2, Qt::SolidLine, Qt::FlatCap, Qt::BevelJoin));

    oldrow = row;
    oldcol = col;
}

void JctEditor::cellDoubleClicked(int row, int col)
{
    // Rotate between the three possible foes,response states (1,1 -> 1,0 -> 0,0 -> 1,1)
    if (foesTable->item(row, col)->text() == "1" && responseTable->item(row, col)->text() == "1")
    {
        foesTable->item(row, col)->setText("1");
        foesTable->item(lanes - col - 1, lanes - row - 1)->setText("1");
        responseTable->item(row, col)->setText("0");
        responseTable->item(lanes - col - 1, lanes - row - 1)->setText("1");
    }
    else if (foesTable->item(row, col)->text() == "1" && responseTable->item(row, col)->text() == "0")
    {
        foesTable->item(row, col)->setText("0");
        foesTable->item(lanes - col - 1, lanes - row - 1)->setText("0");
        responseTable->item(row, col)->setText("0");
        responseTable->item(lanes - col - 1, lanes - row - 1)->setText("0");
    }
    else if (foesTable->item(row, col)->text() == "0" && responseTable->item(row, col)->text() == "0")
    {
        foesTable->item(row, col)->setText("1");
        foesTable->item(lanes - col - 1, lanes - row - 1)->setText("1");
        responseTable->item(row, col)->setText("1");
        responseTable->item(lanes - col - 1, lanes - row - 1)->setText("0");
    }

    // Repaint the tables and junction diagram
    cellChanged(row, col, 0, 0);
}

void JctEditor::applyChanges()
{
    int xmlNo = item->xmlNode;
    QDomElement element = model->getXMLelement(xmlNo);
    QString response, foes;

    for (int i = 0; i < lanes; ++i)
    {
        // Concatenate each cell of the row into a string
        response = ""; foes = "";
        for (int j = 0; j < lanes; ++j)
        {
            response = response + responseTable->item(i, j)->text();
            foes = foes + foesTable->item(i, j)->text();
        }
        // Update the XML element
        for (int k = 0; k < element.childNodes().length(); ++k)
            if (element.childNodes().at(k).nodeName() == "request" && element.childNodes().at(k).toElement().attribute("index") == QString::number(i))
            {
                model->editAttribute(xmlNo, k, "response", response);
                model->editAttribute(xmlNo, k, "foes", foes);
            }
    }
    // Close dialog box
    close();
}
