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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "networkview.h"
#include "propsview.h"
#include "editview.h"
#include "controls.h"

#include <QMainWindow>
#include <QItemSelectionModel>
#include <QSettings>

class Model;
QT_BEGIN_NAMESPACE
class QMenu;
class QTreeView;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    // Constructor
    MainWindow();

    // Selection handler for tree view
    QItemSelectionModel *treeSelections;

protected:
    // Warns if the model has been modified and not saved
    void closeEvent(QCloseEvent *event);

private slots:
    // Creates new model loaded from file
    void openFile();

    // Saves the model
    void saveAsFile();

    // Opens the last saved network in SUMO
    void openSUMO();

    // Opens a dialog to locate SUMO-GUI executable
    void locateSUMO();

    // Opens the Junction Editor, before checking a junction is selected
    void openJunctionEditor();

    // Opens the Signals Editor, before checking a tlLogic is selected
    void openTLEditor();

    // Ensures the item is visible in the tree (when clicked in the network view)
    void scrollTo(QItemSelection on, QItemSelection off);

    // Ensures the item is visible in the network view (when double clicked in the tree)
    void showItem(QModelIndex index);

private:
    // Model instance
    Model *model;
    bool modelLoaded;

    // Menus
    QMenu *fileMenu;
    QMenu *viewMenu;
    QMenu *specialEditorsMenu;
    QIcon nmlJuncIcon;
    QIcon tlLogicIcon;

    // Views and dock widgets
    QTreeView *tView;
    NetworkView *nView;
    PropsView *pView;
    Controls *controls;
    EditView *eView;
    QDockWidget *controlWidget;
    QDockWidget *propsWidget;
    QDockWidget *editWidget;

    // Last path from the File Dialog
    QString xmlPath;

    // Last path from the Locate SUMO Dialog
    QString sumoguiPath;
};

#endif // MAINWINDOW_H
