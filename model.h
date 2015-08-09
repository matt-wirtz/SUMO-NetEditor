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


#ifndef MODEL_H
#define MODEL_H

#include <QAbstractItemModel>
#include <QDomDocument>
#include <QFile>
#include <QModelIndex>
#include <QGraphicsScene>
#include <QItemSelectionModel>
#include <QIcon>
#include <QHash>

class Item;
class PathElement;

class Model : public QAbstractItemModel
{
    Q_OBJECT

public:
    // Constructor and destructor
    explicit Model(QFile *file, QObject *parent = 0);
    ~Model();

    // Element properties
    enum ElementProperty { ViewElement, EditElement, WireElement, ArrowElement };

    // Tree branches
    enum TreeBranch { PJuncsBranch, IJuncsBranch, NEdgesBranch, IEdgesBranch, ConnsBranch, TLBranch };

    // These are the standard reimplementations of QAbstractItemModel necessary to visualise the model in the tree view
    QVariant data(const QModelIndex &index, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    bool hasChildren(const QModelIndex &parent) const;
    QModelIndex index(Item *item);
    
    // Graphics scene where all the PathElements and PointElements are allocated
    // The scene is visualised in the NetworkView
    QGraphicsScene *netScene;

    // Returns the result of QDomDocument.setContent in the constructor
    // true if the XML data was parsed successfully, false otherwise
    bool xmlDataParsed() const;

    // This method links the Selection Model from the Main Window (used for the tree view) into
    // each graphic item in the scene, so that tree view and network view can interact
    void setSelectionModel(QItemSelectionModel *selectionModel);

    // Calls individual loading procedures for junctions, edges, connections, etc.
    void loadModel();

    // Returns if the model has been modified after last saved
    bool wasModified() const;

    // Edits an attribute in the xml document; called either by the edit properties view
    // or by the network view when the shape is changed
    void editAttribute(int node, int subNode, QString attr, QString value);

    // Delete element from model
    void deleteElement(int node);

    // Delete element from model
    void deleteElement(int nodeIndex, int subNodeIndex);

    // Returns an XML element from domDocument
    QDomElement getXMLelement(int index, int subindex = -1);

    // Interprets a link from the Property View and highlights the respective element / point
    void highlightHyperlink(QString link) const;

    // Returns whether the click item is a caption branch or an element
    bool isCaption(Item *item) const;

    // Switches a property of all elements in a layer
    // Called by the Controls View
    void switchLayerState(TreeBranch branch, bool subbranch, Model::ElementProperty prop, bool state) const;
    void switchPointLayerState(TreeBranch branch, Model::ElementProperty prop, bool state) const;

    // used internally when loading model, and by TLEditor
    Item* getJunction(QString id) const;
    Item* getInternalLane(QString id) const;

    // Delete Edge or Lane from scene, the model and the XML SUMO network
    void deleteEdgeAndLane(Item *item);

    // Delete Junction from scene, the model and the XML SUMO network
    void deleteJunction(Item *item);
    
    // Delete Connection from scene, the model and the XML SUMO network
    void deleteConnection(Item *item);
    
    // Save domDocument into a file stream
    void saveTo(QTextStream &textStream);

public slots:
    // Calls deselect() of the 'off' graphic items and select() of the 'on' graphic items
    void selectionChanged(QItemSelection on, QItemSelection off);

signals:
    // Emitted by loadModel() to inform the status of the loading process in the status bar
    void statusUpdate(QString msg);

    // Emitted by editAttribute() so that the properties and edit views are updated
    void attrUpdate(QItemSelection on, QItemSelection off);
    
private:
    // Root item from where 'Plain Junctions', 'Internal Junctions', 'Normal Edges',
    // 'Internal Edges', 'Connections' and 'tlLogics' hang from
    Item *rootItem;

    // Row number for 'Plain Junctions', 'Internal Junctions', 'Normal Edges'
    // 'Connections', etc. captions within the root item
    int pJuncRow, iJuncRow, nEdgeRow, iEdgeRow, connRow, tllRow;

    // XML document handler
    QDomDocument domDocument;

    // Stores if the model has been modified after last saved
    bool modified;

    // Holds the result of QDomDocument.setContent in the constructor
    // true if the XML data was parsed successfully, false otherwise
    bool domDocContentSet;

    // <net> node element number within XML root
    int netNodeIndex;

    // Pointer to the Selection Model of the Main Window
    QItemSelectionModel *itemSelectionModel;

    // Loading procedures
    void loadJunctions();
    void loadEdgesAndLanes();
    void loadConnections();
    void loadSignals();

    // Aiding functions of the loading procedures
    QString getLanePath(QString id) const;
    QString getJunctionXY(QString id) const;

    // This hash is filled in when loading lanes to use when loading connections;
    // after loading connections it is cleared as it is no longer needed
    QHash <QString, QString> laneShapes;

    // Icons for the tree view
    QIcon nmlEdgeIcon;
    QIcon intEdgeIcon;
    QIcon nmlLaneIcon;
    QIcon intLaneIcon;
    QIcon nmlJuncIcon;
    QIcon intJuncIcon;
    QIcon connIcon;
    QIcon tlLogicIcon;
    QIcon requestIcon;
    QIcon phaseIcon;
};

#endif // MODEL_H
