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


#include "mainwindow.h"
#include "model.h"
#include "item.h"
#include "jcteditor.h"
#include "tleditor.h"

#include <QMenuBar>
#include <QStatusBar>
#include <QTreeView>
#include <QDockWidget>
#include <QFileDialog>
#include <QTextStream>
#include <QMessageBox>
#include <QCoreApplication>
#include <QTime>
#include <QProcess>

/* Improvements:
 * control window icons     OK
 * icono tllogic            OK
 * connectors               OK~
 * junction circles         OK
 * intlane zvalues          OK
 * wireframe                OK
 * control panel instances  OK
 * info dock                OK
 * selection                OK
 * tree selection amendment OK
 * tllogic selection        OK
 * status bar with coord!   OK+
 * tab rotation             OK
 * edges: lanes within      OK
 * double click on tree     OK
 * PointElement             OK
 * loading... box           OK
 * setXMLProps in Item      OK
 * prop setting to PathEl   OK
 * rename LinearElement     OK
 * DomDocument to pointer?  NO
 * licence files!           OK
 * app Icon!                OK
 * ordenar+comentar codigo  OK
 * reimp path boundary!     OK
 * junction requests        OK
 * tlLogic phases           OK
 * hypertext labels @ props OK
 * edit!!                   OK
 * edit nodes               OK
 * insert nodes             OK
 * delete nodes             OK
 v0.4
 * arrows                   OK
 * unify switchState        OK
 * dropdowns for editattr   OK
 * junction edit            OK
 * tllogic edit             OK
 * if modified...           OK
 * QMultiHash in Item       OK
 * improve Model interface  OK
 * add consts!!             OK
 * moveTo lineTo warning    OK
 * POINT ELEMENT            OK
 * bug with spacebar        OK
 * splitter                 OK
 * open in SUMO             OK
 * ordenar+comentar codigo  OK
 v0.5
 * delete element
 * add/delete phases (tl)
 * duplicate element!!
 * connectors update??
 * searchXML
 * interpret other files
 * no QColor magic numbers
 * Model::invalidateXMLlines
 * reload xml line numbers
 * Model::refreshXMLlines
 */

MainWindow::MainWindow() : QMainWindow()
{
    // Create network view
    nView = new NetworkView();
    QGraphicsScene *scene = new QGraphicsScene();
    scene->setBackgroundBrush(QBrush(QColor(192, 192, 192)));
    nView->setScene(scene);
    setCentralWidget(nView);

    // Create tree view
    tView = new QTreeView(this);
    QDockWidget *treeDockWidget = new QDockWidget(tr("Tree"), this);
    treeDockWidget->setWidget(tView);
    addDockWidget(Qt::LeftDockWidgetArea, treeDockWidget);

    // Create controls view
    controls = new Controls();
    controlWidget = new QDockWidget(tr("Controls"), this);
    controlWidget->setWidget(controls);
    addDockWidget(Qt::LeftDockWidgetArea, controlWidget);
    controlWidget->hide();

    // Create properties view
    pView = new PropsView(this);
    propsWidget = new QDockWidget(tr("Properties"), this);
    propsWidget->setWidget(pView);
    addDockWidget(Qt::RightDockWidgetArea, propsWidget);
    propsWidget->hide();

    // Create edit view
    eView = new EditView(this);
    editWidget = new QDockWidget(tr("Edit Properties"), this);
    editWidget->setWidget(eView);
    addDockWidget(Qt::RightDockWidgetArea, editWidget);
    editWidget->hide();

    // Create menu
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(tr("&Open..."), this, SLOT(openFile()), QKeySequence::Open);
    fileMenu->addAction(tr("Save &As..."), this, SLOT(saveAsFile()), QKeySequence::Save/*As*/);
    fileMenu->addSeparator();
    fileMenu->addAction(tr("Open in SUMO-GUI"), this, SLOT(openSUMO()));
    fileMenu->addAction(tr("Locate SUMO-GUI..."), this, SLOT(locateSUMO()));
    fileMenu->addSeparator();
    fileMenu->addAction(tr("E&xit"), this, SLOT(close()), QKeySequence::Quit);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(treeDockWidget->toggleViewAction());
    viewMenu->addAction(controlWidget->toggleViewAction());
    viewMenu->addAction(propsWidget->toggleViewAction());
    viewMenu->addAction(editWidget->toggleViewAction());

    specialEditorsMenu = menuBar()->addMenu(tr("&Special Editors"));
    nmlJuncIcon = QPixmap(":/icons/nmlJunc1616.png");
    tlLogicIcon = QPixmap(":/icons/tllogic1616.png");
    specialEditorsMenu->addAction(nmlJuncIcon, tr("&Junction Editor"), this, SLOT(openJunctionEditor()));
    specialEditorsMenu->addAction(tlLogicIcon, tr("&tlLogic Editor"), this, SLOT(openTLEditor()));

    // Read settings from the nefs.ini file
    QSettings settings(QCoreApplication::applicationDirPath() + "/nefs.ini", QSettings::IniFormat);
    xmlPath = settings.value("paths/work").toString();
    sumoguiPath = settings.value("paths/sumo").toString();

    // Status bar
    statusBar()->showMessage(tr("Ready"));

    // Title and icon
    setWindowTitle(tr("Network Editor for SUMO"));
    setWindowIcon(QIcon(QPixmap(":/icons/NE4S128.png")));

    modelLoaded = false;
}

void MainWindow::openFile()
{
    // If the current model has been modified, warn the user before opening the new file
    QMessageBox::StandardButton proceed = QMessageBox::Ok;
    if (modelLoaded)
        if (model->wasModified())
            proceed = QMessageBox::question(this, tr("Network Editor for SUMO"), tr("Model has been modified and not saved. Continue opening a new file?"),
                                            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
    if (proceed == QMessageBox::Ok)
    {
        // Get file name from File Dialog
        QString filePath = QFileDialog::getOpenFileName(this, tr("Open SUMO Network file"),
            xmlPath, tr("XML Network files (*.net.xml)"));
        QCoreApplication::processEvents();

        // Open file
        if (!filePath.isEmpty())
        {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly))
            {
                // Start counting opening time
                QTime t;
                t.start();

                // Create the model, parsing XML data first
                statusBar()->showMessage(tr("Loading XML file..."));
                Model *newModel = new Model(&file, this);

                // If the file has valid XML data, continue loading the model
                if (newModel->xmlDataParsed())
                {
                    // Delete the old model and connect the new one
                    if (modelLoaded) delete model;
                    connect(newModel, SIGNAL(statusUpdate(QString)), statusBar(), SLOT(showMessage(QString)));

                    // Create the item selection model so that it is passed onto the
                    // individual graphic elements as they are created
                    treeSelections = new QItemSelectionModel(newModel);
                    newModel->setSelectionModel(treeSelections);

                    // Interpret XML tree and create the traffic network elements
                    newModel->loadModel();

                    // Connect model with tree view
                    tView->setModel(newModel);
                    tView->setSelectionModel(treeSelections);
                    tView->resizeColumnToContents(0);
                    tView->resizeColumnToContents(1);

                    // Connect model with network view
                    nView->setScene(newModel->netScene);
                    nView->setSelectionModel(treeSelections);
                    nView->zoomExtents();
                    nView->setRenderHint(QPainter::Antialiasing, true);

                    // Connect model with controls and properties view
                    controls->reset();
                    controls->model = newModel;
                    pView->model = newModel;
                    eView->model = newModel;
                    controlWidget->show();
                    propsWidget->show();
                    editWidget->show();

                    // Replace old model by new model
                    model = newModel;
                    modelLoaded = true;
                    xmlPath = filePath;

                    // Update window title
                    QFileInfo fileInfo(file.fileName());
                    QString filename(fileInfo.fileName());
                    setWindowTitle(filename + tr(" - Network Editor for SUMO"));

                    // Connect signals and slots between all views
                    connect(tView, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(showItem(QModelIndex)));
                    connect(nView, SIGNAL(updateStatusBar(QString)), statusBar(), SLOT(showMessage(QString)));
                    connect(treeSelections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), model, SLOT(selectionChanged(QItemSelection, QItemSelection)));
                    connect(treeSelections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), pView, SLOT(selectionChanged(QItemSelection, QItemSelection)));
                    connect(treeSelections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), eView, SLOT(selectionChanged(QItemSelection, QItemSelection)));
                    connect(treeSelections, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), this, SLOT(scrollTo(QItemSelection, QItemSelection)));
                    connect(model, SIGNAL(attrUpdate(QItemSelection, QItemSelection)), pView, SLOT(selectionChanged(QItemSelection, QItemSelection)));
                    connect(model, SIGNAL(attrUpdate(QItemSelection, QItemSelection)), eView, SLOT(selectionChanged(QItemSelection, QItemSelection)));
                    statusBar()->showMessage(tr("Ready. Model loaded in %1ms.").arg(t.elapsed()));
                }
                else
                {
                    // The XML data in the file was not valid, so do not load the model
                    delete newModel;
                    QMessageBox::warning(this, tr("Network Editor for SUMO"), tr("Error parsing XML data."));
                }
                // Close file
                file.close();
            }
        }
    }
}

void MainWindow::saveAsFile()
{
    if (modelLoaded)
    {
        // Get file name from File Dialog
        QString filePath = QFileDialog::getSaveFileName(this, tr("Save SUMO Network file as"),
            xmlPath, tr("XML Network files (*.net.xml)"));
        QCoreApplication::processEvents();

        // Open file and save model domDocument in it
        if (!filePath.isEmpty())
        {
            QFile file(filePath);
            statusBar()->showMessage(tr("Saving XML file..."));
            if (file.open(QIODevice::WriteOnly | QIODevice::Text))
            {
                QTextStream textStream(&file);
                model->saveTo(textStream);

                xmlPath = filePath;
                QFileInfo fileInfo(file.fileName());
                QString filename(fileInfo.fileName());
                setWindowTitle(filename + tr(" - Network Editor for SUMO"));
            }
            file.close();
            statusBar()->showMessage(tr("Ready"));
        }
    } else {
        QMessageBox::information(this, tr("Save As..."), tr("No model loaded to save."));
    }
}

void MainWindow::scrollTo(QItemSelection on, QItemSelection off)
{
    // Ensure the item is visible in the tree (when clicked in the network view)
    Q_UNUSED(off);
    if ( !on.indexes().empty() ) {
        tView->scrollTo(on.indexes()[0], QAbstractItemView::EnsureVisible);
    }
}

void MainWindow::showItem(QModelIndex index)
{
    // Ensure the item is visible in the network view (when double clicked in the tree)
    Item *item = static_cast<Item*>(index.internalPointer());
    if (item->hasPath)
        nView->centerOn(item->graphicItem1);
    else if (item->hasPoint)
        nView->centerOn(item->graphicItem2);
}

void MainWindow::openJunctionEditor()
{
    bool success = false;
    if (modelLoaded && treeSelections->hasSelection())
        if (treeSelections->selection().indexes().count() > 0)
            if (treeSelections->selection().indexes()[0].isValid())
            {
                // Find selected item pointer; if it is a request, use the parent junction
                Item *itemOn = static_cast<Item*>(treeSelections->selection().indexes()[0].internalPointer());
                if (itemOn->type == Item::Request) itemOn = itemOn->parent();
                if (itemOn->type == Item::Junction)
                {
                    // Open the junction editor
                    JctEditor jctDialog(this, model, itemOn);
                    jctDialog.exec();
                    success = true;
                }
            }
    if (!success) QMessageBox::information(this, tr("Network Editor for SUMO"), tr("Select a plain junction first."));
}

void MainWindow::openTLEditor()
{
    bool success = false;
    if (modelLoaded && treeSelections->hasSelection())
        if (treeSelections->selection().indexes().count() > 0)
            if (treeSelections->selection().indexes()[0].isValid())
            {
                // Find selected item pointer; if it is a phase, use the parent tlLogic
                Item *itemOn = static_cast<Item*>(treeSelections->selection().indexes()[0].internalPointer());
                if (itemOn->type == Item::Phase) itemOn = itemOn->parent();
                if (itemOn->type == Item::tlLogic)
                {
                    // Open the tlLogic editor
                    TLEditor tlDialog(this, model, itemOn);
                    tlDialog.exec();
                    success = true;
                }
            }
    if (!success) QMessageBox::information(this, tr("Network Editor for SUMO"), tr("Select a Traffic Lights program first."));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    // If the current model has been modified, warn the user before closing
    QMessageBox::StandardButton proceed = QMessageBox::Ok;
    if (modelLoaded)
        if (model->wasModified())
            proceed = QMessageBox::question(this, tr("Network Editor for SUMO"), tr("Model has been modified and not saved. Continue closing?"),
                                            QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);

    if (proceed == QMessageBox::Ok)
        // Close the main window;
        close();
    else
        // Ignore closing event
        event->ignore();

    // Write settings into the nefs.ini file
    QSettings settings(QCoreApplication::applicationDirPath() + "/nefs.ini", QSettings::IniFormat);
    settings.setValue("paths/work", xmlPath.left(xmlPath.lastIndexOf('/')));
    settings.setValue("paths/sumo", sumoguiPath);
}

void MainWindow::openSUMO()
{
    if (sumoguiPath != "")
    {
        // Start a new process with SUMO-GUI
        QStringList arguments;
        arguments << "-n" << xmlPath;

        QProcess *sumoProcess = new QProcess(this);
        sumoProcess->start(sumoguiPath, arguments);
    }
    else
        QMessageBox::information(this, tr("Network Editor for SUMO"), tr("Locate SUMO-GUI first."));
}

void MainWindow::locateSUMO()
{
    // Open an open file dialog window and if the return path is not empty, update sumoguiPath
    QString filePath = QFileDialog::getOpenFileName(this, tr("Locate SUMO-GUI"), sumoguiPath, tr("*.*"));
    if (!filePath.isEmpty())
        sumoguiPath = filePath;
}
