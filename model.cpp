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


#include "model.h"
#include "item.h"
#include "pathelement.h"
#include "pointelement.h"

#include <QtXml>
#include <QDebug>
#include <QDomNode>
#include <QMessageBox>

Model::Model(QFile *file, QObject *parent) : QAbstractItemModel(parent)
{
    // Load the XML data in the file and store in domDocContentSet if successful
    domDocContentSet = domDocument.setContent(file);

    // Create a root item
    rootItem = new Item("Model", 0);

    // Load icons
    nmlEdgeIcon = QPixmap(":/icons/edge1616.png");
    intEdgeIcon = QPixmap(":/icons/intEdge1616.png");
    nmlLaneIcon = QPixmap(":/icons/nmlLane1616.png");
    intLaneIcon = QPixmap(":/icons/intLane1616.png");
    nmlJuncIcon = QPixmap(":/icons/nmlJunc1616.png");
    intJuncIcon = QPixmap(":/icons/intJunc1616.png");
    connIcon = QPixmap(":/icons/connection1616.png");
    tlLogicIcon = QPixmap(":/icons/tllogic1616.png");
    requestIcon = QPixmap(":/icons/request1616.png");
    phaseIcon = QPixmap(":/icons/phase1616.png");

    // Create a graphics scene
    netScene = new QGraphicsScene();
    netScene->setBackgroundBrush(QBrush(QColor(192, 192, 192)));

    //loadModel();
    // The model is not loaded from the constructor so that the signals (emmited by the model when loading) can be
    // connected to the status bar slots. The connection is made in MainWindow::openFile() and then the model is loaded

    modified = false;
}

Model::~Model()
{
    delete rootItem;
}

int Model::columnCount(const QModelIndex &/*parent*/) const
{
    // Implementation required by QAbstractItemModel
    return 2;
}

QVariant Model::data(const QModelIndex &index, int role) const
{
    // Implementation required by QAbstractItemModel
    if (!index.isValid())
        return QVariant();

    if (role != Qt::DisplayRole && role != Qt::DecorationRole)
        return QVariant();

    Item *item = static_cast<Item*>(index.internalPointer());
    if (role == Qt::DisplayRole && index.column() == 0) return QVariant(item->name);
    if (role == Qt::DisplayRole && index.column() == 1 && item->xmlLine > 0) return QVariant(item->xmlLine);
    if (role == Qt::DecorationRole && index.column() == 0)
    {
        switch (item->iconType)
        {
            case 1: return QVariant::fromValue(nmlEdgeIcon);
            case 2: return QVariant::fromValue(intEdgeIcon);
            case 3: return QVariant::fromValue(nmlLaneIcon);
            case 4: return QVariant::fromValue(intLaneIcon);
            case 5: return QVariant::fromValue(nmlJuncIcon);
            case 6: return QVariant::fromValue(intJuncIcon);
            case 7: return QVariant::fromValue(connIcon);
            case 8: return QVariant::fromValue(tlLogicIcon);
            case 9: return QVariant::fromValue(requestIcon);
            case 10: return QVariant::fromValue(phaseIcon);
        }
    }
    return QVariant();
}

Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
    // Implementation required by QAbstractItemModel
    if (!index.isValid())
        return 0;

    return QAbstractItemModel::flags(index);
}

QVariant Model::headerData(int section, Qt::Orientation orientation,
                              int role) const
{
    // Implementation required by QAbstractItemModel
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole) {
        switch (section) {
            case 0:
                return tr("Item");
            case 1:
                return tr("XML Line");
            default:
                return QVariant();
        }
    }
    return QVariant();
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
    // Implementation required by QAbstractItemModel
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    Item *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Item*>(parent.internalPointer());

    Item *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}


QModelIndex Model::index(Item *item)
{
  return createIndex(item->row(), 0, item);
}

  
QModelIndex Model::parent(const QModelIndex &child) const
{
    // Implementation required by QAbstractItemModel
    if (!child.isValid())
        return QModelIndex();

    Item *childItem = static_cast<Item*>(child.internalPointer());
    Item *parentItem = childItem->parent();

    if (!parentItem || parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

int Model::rowCount(const QModelIndex &parent) const
{
    // Implementation required by QAbstractItemModel
    if (parent.column() > 0)
        return 0;

    Item *parentItem;

    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Item*>(parent.internalPointer());

    return parentItem->childCount();
}

bool Model::hasChildren(const QModelIndex &parent) const
{
    // Implementation required by QAbstractItemModel
    Item *parentItem;
    if (!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<Item*>(parent.internalPointer());

    return parentItem->childCount() > 0;
}

void Model::loadModel()
{
    // Find <net> node number
    for (int i = 0; i < domDocument.childNodes().count(); ++i)
        if (domDocument.childNodes().at(i).nodeName() == "net")
        {
            netNodeIndex = i;
            break;
        }

    // Load elements
    emit statusUpdate(tr("Loading XML file: Junctions..."));
    loadJunctions();
    emit statusUpdate(tr("Loading XML file: Edges and Lanes..."));
    loadEdgesAndLanes();
    emit statusUpdate(tr("Loading XML file: Connections..."));
    loadConnections();
    emit statusUpdate(tr("Loading XML file: Traffic signals..."));
    loadSignals();
}

void Model::loadJunctions()
{
    // Add parent nodes for plain and internal junctions to the tree
    Item *pjunc = new Item(tr("Plain Junctions"), 5, rootItem);
    pJuncRow = rootItem->appendChild(pjunc);
    QModelIndex pIndex = index(pJuncRow, 0);

    Item *ijunc = new Item(tr("Internal Junctions"), 6, rootItem);
    iJuncRow = rootItem->appendChild(ijunc);
    QModelIndex iIndex = index(iJuncRow, 0);

    QDomElement element;
    QString shape, xs, ys;
    int newRow;
    bool internal;

    // Scan all "junction" elements in <net>
    QDomNodeList netNode = domDocument.childNodes().at(netNodeIndex).childNodes();
    for (unsigned int i = 0; i < netNode.length(); ++i)
        if (netNode.at(i).nodeName() == "junction")
        {
            element = netNode.at(i).toElement();

            internal = (element.attribute("type") == "internal" ? true : false);

            // Create model item (plain or internal) and set XML file links and props
            Item *juncItem = new Item(element.attribute("id"), (internal ? 6 : 5), (internal ? ijunc : pjunc));
            juncItem->setXMLdata(Item::Junction, element.lineNumber(), i, -1);
            newRow = (internal ? ijunc->appendChild(juncItem) : pjunc->appendChild(juncItem));

            // Determine junction geometry and add to the scene
            shape = element.attribute("shape").trimmed();
            xs = element.attribute("x").trimmed();
            ys = element.attribute("y").trimmed();
            if ((shape != "") || (xs != "" && ys !=""))
            {
                // Junction polygon
                if (shape != "")
                {
                    PathElement *pathItem = new PathElement((internal ? PathElement::IntJunction : PathElement::PlainJunction), shape, this, juncItem, itemSelectionModel);
                    netScene->addItem(pathItem);

                    // Link the graphic element to the model item
                    juncItem->graphicItem1 = pathItem;
                    juncItem->hasPath = true;

                    // Link the model item to the graphic element
                    pathItem->modelIndex = index(newRow, 0, (internal ? iIndex : pIndex));
                }

                // Junction XY point
                if (xs != "" && ys !="")
                {
                    PointElement *pointItem = new PointElement((internal ? PointElement::IntJunction : PointElement::PlainJunction),
                        xs.toDouble(), ys.toDouble(), this, juncItem, itemSelectionModel);
                    netScene->addItem(pointItem);

                    // Link the graphic element to the model item
                    juncItem->graphicItem2 = pointItem;
                    juncItem->hasPoint = true;

                    // Link the model item to the graphic element
                    pointItem->modelIndex = index(newRow, 0, (internal ? iIndex : pIndex));

                    // Create QPointF, needed when loading edges with no 'shape' attribute
                    juncItem->junctionXY = xs + QString(",") + ys;
                }

                // Scan requests within junction
                for (unsigned int j = 0; j < netNode.at(i).childNodes().length(); ++j)
                    if (netNode.at(i).childNodes().at(j).nodeName() == "request")
                    {
                        element = netNode.at(i).childNodes().at(j).toElement();

                        // Create model item and set XML file links and props
                        Item *reqItem = new Item("reqst " + element.attribute("index"), 9, juncItem);
                        reqItem->setXMLdata(Item::Request, element.lineNumber(), i, j);
                        juncItem->appendChild(reqItem);
                    }
            }
        }
}

void Model::loadEdgesAndLanes()
{
    // Add parent nodes for normal and internal edges to the tree
    Item *nedges = new Item(tr("Normal Edges"), 1, rootItem);
    nEdgeRow = rootItem->appendChild(nedges);
    QModelIndex neIndex = index(nEdgeRow, 0);

    Item *iedges = new Item(tr("Internal Edges"), 2, rootItem);
    iEdgeRow = rootItem->appendChild(iedges);
    QModelIndex ieIndex = index(iEdgeRow, 0);

    QDomElement element;
    QString id, shape, from, to, a, b;
    PathElement *pathItem;
    QModelIndex edgeIndex;
    int newRow;
    bool internal;

    laneShapes.clear();

    // Scan all "edge" elements in <net>
    QDomNodeList netNode = domDocument.childNodes().at(netNodeIndex).childNodes();
    for (unsigned int i = 0; i < netNode.length(); ++i)
        if (netNode.at(i).nodeName() == "edge")
        {
            element = netNode.at(i).toElement();

            internal = (element.attribute("function") == "internal" ? true : false);

            // Create model item (normal or internal) and set XML file links and props
            Item *edgeItem = new Item(element.attribute("id"), (internal ? 2 : 1), (internal ? iedges : nedges));
            edgeItem->setXMLdata(Item::Edge, element.lineNumber(), i, -1);
            newRow = (internal ? iedges->appendChild(edgeItem) : nedges->appendChild(edgeItem));

            // Determine edge geometry and add to the scene
            shape = element.attribute("shape").trimmed();
            from = element.attribute("from");
            to =  element.attribute("to");
            if (shape != "")
            {
                pathItem = new PathElement(PathElement::Edge, shape, this, edgeItem, itemSelectionModel);
                netScene->addItem(pathItem);

                // Link the graphic element to the model item
                edgeItem->graphicItem1 = pathItem;
                edgeItem->hasPath = true;
            }
            else if (from != "" && to != "")
            {
                a = getJunctionXY(from);
                b = getJunctionXY(to);
                pathItem = new PathElement(PathElement::EdgeNoShape, a + QString(" ") + b, this, edgeItem, itemSelectionModel);
                netScene->addItem(pathItem);

                // Link the graphic element to the model item
                edgeItem->graphicItem1 = pathItem;
                edgeItem->hasPath = true;
            }

            // Link the model item to the graphic element
            edgeIndex = index(newRow, 0, (internal ? ieIndex : neIndex));
            if (edgeItem->hasPath) pathItem->modelIndex = edgeIndex;

            // Scan lanes within edge
            for (unsigned int j = 0; j < netNode.at(i).childNodes().length(); ++j)
                if (netNode.at(i).childNodes().at(j).nodeName() == "lane")
                {
                    element = netNode.at(i).childNodes().at(j).toElement();

                    // Create model item and set XML file links and props
                    id = element.attribute("id");
                    Item *laneItem = new Item(id, (internal ? 4 : 3), edgeItem);
                    laneItem->setXMLdata(Item::Lane, element.lineNumber(), i, j);
                    newRow = edgeItem->appendChild(laneItem);

                    // Parse lane geometry and add to the scene
                    shape = element.attribute("shape").trimmed();
                    if (shape != "")
                    {
                        pathItem = new PathElement((internal ? PathElement::IntLane : PathElement::NormalLane), shape, this, laneItem, itemSelectionModel);
                        netScene->addItem(pathItem);

                        // Link the graphic element to the model item
                        laneItem->graphicItem1 = pathItem;
                        laneItem->hasPath = true;

                        // Add the id and shape of the lane into laneShape to use when loading connections
                        laneShapes.insert(id, shape);
                    }

                    // Link the model item to the graphic element
                    if (laneItem->hasPath) pathItem->modelIndex = index(newRow, 0, edgeIndex);
                }
        }
}

void Model::loadConnections()
{
    // Add parent node to the tree
    Item *conn = new Item(tr("Connections"), 7, rootItem);
    connRow = rootItem->appendChild(conn);
    QModelIndex cIndex = index(connRow, 0);

    QDomElement element;
    QString fromLane, toLane, viaLane, connectorPath, fromPath, toPath;
    QStringList tokens;
    QPointF a, b, c;
    bool addPoint;
    int newRow;

    // Scan all "connection" elements in <net>
    QDomNodeList netNode = domDocument.childNodes().at(netNodeIndex).childNodes();
    for (unsigned int i = 0; i < netNode.length(); ++i)
        if (netNode.at(i).nodeName() == "connection")
        {
            element = netNode.at(i).toElement();

            fromLane = element.attribute("from") + QString("_") + element.attribute("fromLane");
            toLane = element.attribute("to") + QString("_") + element.attribute("toLane");

            // Create model item and set XML file links and props
            Item *connItem = new Item(fromLane + QString(" -> ") + toLane, 7, conn);
            connItem->setXMLdata(Item::Connection, element.lineNumber(), i, -1);
            newRow = conn->appendChild(connItem);

            // Determine connection geometry and add to the scene
            viaLane = element.attribute("via");
            addPoint = false;
            if (viaLane != "")
                connectorPath = getLanePath(viaLane);
            else
            {
                fromPath = getLanePath(fromLane);
                fromPath.replace(" ", ",");
                tokens = fromPath.split(",");
                a = QPointF(tokens[tokens.length() - 2].toDouble(), tokens[tokens.length() - 1].toDouble());

                toPath = getLanePath(toLane);
                toPath.replace(" ", ",");
                tokens = toPath.split(",");
                b = QPointF(tokens[0].toDouble(), tokens[1].toDouble());

                connectorPath = QString::number(a.x(), 'f', 2) + QString(",") + QString::number(a.y(), 'f', 2) + QString(" ")
                              + QString::number(b.x(), 'f', 2) + QString(",") + QString::number(b.y(), 'f', 2);

                // if the start and end point of the connector are the same, add a point element
                if (a == b)
                {
                    addPoint = true;
                    c = a;
                }
            }

            // If the geometry of the connection is only a point, add a PointElement
            if (addPoint)
            {
                PointElement *pointItem = new PointElement(PointElement::Connection, c.x(), c.y(), this, connItem, itemSelectionModel);
                netScene->addItem(pointItem);

                // Link the graphic element to the model item
                connItem->graphicItem2 = pointItem;
                connItem->hasPoint = true;

                // Link the model item to the graphic element
                pointItem->modelIndex = index(newRow, 0, cIndex);
            } else {
                // Connection has a geometry -> add as a PathElement
                PathElement *pathItem = new PathElement(PathElement::Connection, connectorPath, this, connItem, itemSelectionModel);
                netScene->addItem(pathItem);

                // Link the graphic element to the model item
                connItem->graphicItem1 = pathItem;
                connItem->hasPath = true;

                // Link the model item to the graphic element
                pathItem->modelIndex = index(newRow, 0, cIndex);
            }

        }
    // laneShapes hash is no longer needed
    laneShapes.clear();
}

void Model::loadSignals()
{
    // Add parent node to the tree
    Item *tlLogics = new Item(tr("Traffic Lights"), 8, rootItem);
    tllRow = rootItem->appendChild(tlLogics);

    QDomElement element;
    Item *junction;

    // Scan all "tlLogic" elements in <net>
    QDomNodeList netNode = domDocument.childNodes().at(netNodeIndex).childNodes();
    for (unsigned int i = 0; i < netNode.length(); ++i)
        if (netNode.at(i).nodeName() == "tlLogic")
        {
            element = netNode.at(i).toElement();

            // Create model item and set XML file links and props
            Item *logicItem = new Item(element.attribute("id"), 8, tlLogics);
            logicItem->setXMLdata(Item::tlLogic, element.lineNumber(), i, -1);
            tlLogics->appendChild(logicItem);

            // Find associated junction and copy pointers to graphic elements
            junction = getJunction(element.attribute("id"));
            if (junction != NULL)
            {
                logicItem->hasPath = junction->hasPath;
                logicItem->graphicItem1 = junction->graphicItem1;
                logicItem->hasPoint = junction->hasPoint;
                logicItem->graphicItem2 = junction->graphicItem2;
            }

            // Scan phases within logic
            for (unsigned int j = 0; j < netNode.at(i).childNodes().length(); ++j)
                if (netNode.at(i).childNodes().at(j).nodeName() == "phase")
                {
                    element = netNode.at(i).childNodes().at(j).toElement();

                    // Create model item and set XML file links and props
                    Item *phaseItem = new Item("phase " + QString::number(j), 10, logicItem);
                    phaseItem->setXMLdata(Item::Phase, element.lineNumber(), i, j);
                    logicItem->appendChild(phaseItem);
                }
        }
}

QString Model::getLanePath(QString id) const
{
    // Get lane path from the QMultiHash using its id
    if (laneShapes.contains(id))
        return laneShapes.value(id);
    else
        return "";
}

Item *Model::getJunction(QString id) const
{
    // Get a reference to the junction item using its id
    return rootItem->child(pJuncRow)->child(id);
}

QString Model::getJunctionXY(QString id) const
{
    // Attempt to find the junction in the plain junctions first
    Item *item = rootItem->child(pJuncRow)->child(id);
    if (item != NULL) return item->junctionXY;

    // And in the internal junctions secondly
    item = rootItem->child(iJuncRow)->child(id);
    if (item != NULL) return item->junctionXY;

    // Otherwise return an empty point
    return "";
}

Item *Model::getInternalLane(QString id) const
{
    // Determine the edge name by removing _ and all it is after it
    QString edgeName = id.left(id.lastIndexOf("_"));
    // Get the internal lane from the edge
    return rootItem->child(iEdgeRow)->child(edgeName)->child(id);
}

void Model::selectionChanged(QItemSelection on, QItemSelection off)
{
    Item *itemOn, *itemOff;

    // Deselect item that was previously selected
    if (off.indexes().count() > 0)
        if (off.indexes()[0].isValid())
        {
            itemOff = static_cast<Item*>(off.indexes()[0].internalPointer());
            if (itemOff->parent() != rootItem && itemOff->hasPath)
                itemOff->graphicItem1->deselect();
            if (itemOff->parent() != rootItem && itemOff->hasPoint)
                itemOff->graphicItem2->deselect();
        }

    // Select the new item
    if (on.indexes().count() > 0)
        if (on.indexes()[0].isValid())
        {
            itemOn = static_cast<Item*>(on.indexes()[0].internalPointer());
            qDebug() << "Model::selectionChanged on signal for item name: " << itemOn->name;
            if (itemOn->parent() != rootItem && itemOn->hasPath)
                itemOn->graphicItem1->select();
            if (itemOn->parent() != rootItem && itemOn->hasPoint)
                itemOn->graphicItem2->select();
        }
}

void Model::setSelectionModel(QItemSelectionModel *selectionModel)
{
    // Set the item selection model
    itemSelectionModel = selectionModel;
}

void Model::editAttribute(int node, int subNode, QString attr, QString value)
{
    // Update the XML domDocument attribute
    if (subNode > -1)
        domDocument.childNodes().at(netNodeIndex).childNodes().at(node).childNodes().at(subNode)
                .toElement().setAttribute(attr, value);
    else
        domDocument.childNodes().at(netNodeIndex).childNodes().at(node)
                .toElement().setAttribute(attr, value);

    modified = true;

    // Emit a signal so that the properties view is updated
    emit attrUpdate(itemSelectionModel->selection(), itemSelectionModel->selection());
}

void Model::deleteElement(int nodeIndex)
{
    qDebug() << "Model::deleteElement: nodeIndex=" << QString::number(nodeIndex);
    QDomNode node = domDocument.childNodes().item(netNodeIndex).childNodes().item(nodeIndex).toElement();
    node.parentNode().removeChild(node);

    modified = true;
    
    // Emit a signal so that the properties view is updated
    emit attrUpdate(itemSelectionModel->selection(), itemSelectionModel->selection());
}

void Model::deleteElement(int nodeIndex, int subNodeIndex)
{
    qDebug("Model: deleteElement");
    QDomNode node = domDocument.childNodes().item(netNodeIndex).childNodes().item(nodeIndex).childNodes().item(subNodeIndex).toElement();
    node.parentNode().removeChild(node);

    modified = true;
    
    // Emit a signal so that the properties view is updated
    emit attrUpdate(itemSelectionModel->selection(), itemSelectionModel->selection());
}

bool Model::isCaption(Item *item) const
{
    // If the parent of the selected item is the root item, it is a first level branch
    // (a caption) and not an item
    return item->parent() == rootItem;
}

void Model::highlightHyperlink(QString link) const
{
    // Split the link into the prefix and suffix
    QString prefix = link.left(2);
    QString suffix = link.remove(0, 2);

    // Highlight the element according to the prefix
    if (prefix == "1/")  // edge->to or edge->from
    {
        if (rootItem->child(pJuncRow)->child(suffix)->hasPath)
            rootItem->child(pJuncRow)->child(suffix)->graphicItem1->highlight();
        if (rootItem->child(pJuncRow)->child(suffix)->hasPoint)
            rootItem->child(pJuncRow)->child(suffix)->graphicItem2->highlight();

        return;
    }
    if (prefix == "3/" || prefix == "9/")  // junction->incLanes or connection->toFromLanes
    {
        QString edgeName = suffix.left(suffix.lastIndexOf("_"));
        if (rootItem->child(nEdgeRow)->child(edgeName) != NULL)
            if (rootItem->child(nEdgeRow)->child(edgeName)->child(suffix)->hasPath)
            {
                rootItem->child(nEdgeRow)->child(edgeName)->child(suffix)->graphicItem1->highlight();
                return;
            }
    }
    if (prefix == "4/" || prefix == "9/")  // junction->intLanes or connection->via
    {
        QString edgeName = suffix.left(suffix.lastIndexOf("_"));
        if (rootItem->child(iEdgeRow)->child(edgeName) != NULL)
            if (rootItem->child(iEdgeRow)->child(edgeName)->child(suffix)->hasPath)
            {
                rootItem->child(iEdgeRow)->child(edgeName)->child(suffix)->graphicItem1->highlight();
                return;
            }
    }
    if (prefix == "5/")  // junction->shape (point in the shape)
    {
        int pointBreak = suffix.lastIndexOf("/");
        QString id = suffix.left(pointBreak);
        int pointNo = suffix.remove(0, pointBreak + 1).toInt();
        if (rootItem->child(pJuncRow)->child(id)->hasPath)
            rootItem->child(pJuncRow)->child(id)->graphicItem1->highlightPoint(pointNo);
    }
    if (prefix == "6/")  // edge->shape (point in the shape)
    {
        int pointBreak = suffix.lastIndexOf("/");
        QString id = suffix.left(pointBreak);
        int pointNo = suffix.remove(0, pointBreak + 1).toInt();
        if (rootItem->child(nEdgeRow)->child(id)->hasPath)
            rootItem->child(nEdgeRow)->child(id)->graphicItem1->highlightPoint(pointNo);
    }
    if (prefix == "7/")  // lane->shape (point in the shape)
    {
        int pointBreak = suffix.lastIndexOf("/");
        QString id = suffix.left(pointBreak);
        int pointNo = suffix.remove(0, pointBreak + 1).toInt();
        QString edgeName = id.left(id.lastIndexOf("_"));
        if (rootItem->child(nEdgeRow)->child(edgeName) != NULL)
            if (rootItem->child(nEdgeRow)->child(edgeName)->child(id)->hasPath)
            {
                rootItem->child(nEdgeRow)->child(edgeName)->child(id)->graphicItem1->highlightPoint(pointNo);
                return;
            }
        if (rootItem->child(iEdgeRow)->child(edgeName) != NULL)
            if (rootItem->child(iEdgeRow)->child(edgeName)->child(id)->hasPath)
            {
                rootItem->child(iEdgeRow)->child(edgeName)->child(id)->graphicItem1->highlightPoint(pointNo);
                return;
            }
    }
}

void Model::switchLayerState(TreeBranch branch, bool subbranch, ElementProperty prop, bool state) const
{
    // Identify the branch number
    int branchNumber;
    switch (branch)
    {
        case PJuncsBranch: branchNumber = pJuncRow; break;
        case IJuncsBranch: branchNumber = iJuncRow; break;
        case NEdgesBranch: branchNumber = nEdgeRow; break;
        case IEdgesBranch: branchNumber = iEdgeRow; break;
        case ConnsBranch:  branchNumber = connRow;  break;
        default: branchNumber = -1; // error
    }
    // Send the switchState message to all the elemets of the branch, and subbranches if applies
    if (!subbranch)
    {
        // sublayer = false is used for edge, junction and connector items
        // Sweep all items within the tree node/layer and switch on/off the property if they have a path element
        for (int i = 0; i < rootItem->child(branchNumber)->childCount(); ++i)
            if (rootItem->child(branchNumber)->child(i)->hasPath)
                rootItem->child(branchNumber)->child(i)->graphicItem1->switchState(prop, state);
    } else {
        // sublayer = true is used for lane items within edge items
        // Sweep all items within the tree node/layer and switch on/off the property if they have a path element
        for (int i = 0; i < rootItem->child(branchNumber)->childCount(); ++i)
            for (int j = 0; j < rootItem->child(branchNumber)->child(i)->childCount(); ++j)
                if (rootItem->child(branchNumber)->child(i)->child(j)->hasPath)
                    rootItem->child(branchNumber)->child(i)->child(j)->graphicItem1->switchState(prop, state);
    }
}

void Model::switchPointLayerState(TreeBranch branch, ElementProperty prop, bool state) const
{
    // Identify the branch number
    int branchNumber;
    switch (branch)
    {
        case PJuncsBranch: branchNumber = pJuncRow; break;
        case IJuncsBranch: branchNumber = iJuncRow; break;
        case ConnsBranch: branchNumber = connRow; break;
        default: branchNumber = -1; // error
    }
    // Sweep all items within the tree node/layer and switch on/off the property if they have a point element
    for (int i = 0; i < rootItem->child(branchNumber)->childCount(); ++i)
        if (rootItem->child(branchNumber)->child(i)->hasPoint)
            rootItem->child(branchNumber)->child(i)->graphicItem2->switchState(prop, state);
}

QDomElement Model::getXMLelement(int index, int subindex)
{
    // Get the XML element from domDocument
    if (subindex == -1)
        return domDocument.childNodes().at(netNodeIndex).childNodes().at(index).toElement();
    else
        return domDocument.childNodes().at(netNodeIndex).childNodes().at(index).toElement().childNodes().at(subindex).toElement();
}

void Model::saveTo(QTextStream &textStream)
{
    // Save the domDocument into the file
    //stream << model->domDocument.toString();
    int indent = 4;
    domDocument.save(textStream, indent);
    modified = false;
}

bool Model::wasModified() const
{
    return modified;
}

bool Model::xmlDataParsed() const
{
    return domDocContentSet;
}

void Model::deleteEdgeAndLane(Item *item)
{
    // the functionality to remove objects from the scene should rather be part of the pathelement class
    // but these elements are added here in the model class
    QDomElement element;
    PathElement *pathit;
    Item *parent_item, *grand_parent_item;
    
    qDebug() << "Model::deleteEdgeAndLane, name: " << item->name;
    
    if ( item->hasPath ) {
        pathit = item->graphicItem1;
    } else {
        QMessageBox::information(NULL, "Model", "Deletion of a non path element not yet implemented!");
    }

    int nodeIndex = item->xmlNode;
    int subNodeIndex = item->xmlSubNode;
    
    // check whether a subnode exists for this node. This holds true for lanes!
    if (subNodeIndex == -1) {
        element = domDocument.childNodes().item(netNodeIndex).childNodes().item(nodeIndex).toElement();
    } else {
        element = domDocument.childNodes().item(netNodeIndex).childNodes().item(nodeIndex).childNodes().item(subNodeIndex).toElement();
    }
    
    qDebug() << "Model::deleteEdgeAndLane: XML document name: " << element.attribute("id");
    qDebug() << "Model::deleteEdgeAndLane: ElementType: " << QString::number(pathit->type);
    qDebug() << "Model::deleteEdgeAndLane: modelIndex->row: " << QString::number(pathit->modelIndex.row());
    qDebug() << "Model::deleteEdgeAndLane: modelIndex->col: " << QString::number(pathit->modelIndex.column());
    qDebug() << "Model::deleteEdgeAndLane: item row: " << QString::number(item->row());
    qDebug() << "Model::deleteEdgeAndLane: item parent name: " << item->parent()->name;
    qDebug() << "Model::deleteEdgeAndLane: item parent hasPath: " << item->parent()->hasPath;
    qDebug() << "Model::deleteEdgeAndLane: parent modelIndex->row: " << pathit->modelIndex.parent().row();
    qDebug() << "Model::deleteEdgeAndLane: itemSelection before clear: " << itemSelectionModel->selection().indexes().empty();
    itemSelectionModel->clearSelection();
    
    beginResetModel();
    
    qDebug() << "Model: deleteEdgeAndLane, itemSelection after clear: " << itemSelectionModel->selection().indexes().empty();
    //netScene->clearSelection();
    netScene->removeItem(pathit);
    // clear mem
    // clear mem creates a problem since we were called from the graphicItem itself
    // and need to be able to return to it.
    // Maybe move deleted item to a save place and delete later?
    //delete(pathit);
    
    //qDebug() << "Model: deleteEdgeAndLane, pathit: " << QString("0x%1").arg((quintptr)pathit, QT_POINTER_SIZE * 2, 16, QChar('0'));;

    // remove Element from model structure
    parent_item = item->parent();
    parent_item->removeChild(item);
    
    // remove Element from XML structure
    deleteElement(nodeIndex, subNodeIndex);

    qDebug() << "Model: deleteEdgeAndLane, no of childs left: " << QString::number(parent_item->childCount());

    QMessageBox::StandardButton yes_button = QMessageBox::Yes;
    yes_button = QMessageBox::question(NULL, tr("Network Editor for SUMO"), tr("Last lane will be deleted. Should the edge be deleted too?"),
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Cancel);

    if (yes_button == QMessageBox::Yes) {
        // Delete edge
        qDebug() << "Model: deleteEdgeAndLane, delete edge: " << parent_item->name;

        // if the parent item has a path then remove path from scene
        if ( parent_item->hasPath ) {
            netScene->removeItem(parent_item->graphicItem1);
            // clear mem creates a problem since we were called from the graphicItem itself
            // and need to be able to return to it.
            // Maybe move deleted item to a save place and delete later?
            //delete(parent_item->graphicItem1);

        }

        //remove Element from model
        grand_parent_item = parent_item->parent();
        grand_parent_item->removeChild(parent_item);

        // remove Element from XML structure
        deleteElement(nodeIndex);

    }    
        
    endResetModel();    
}

void Model::deleteJunction(Item *item)
{
    PathElement *pathit;
    PointElement *pointit;
    
    qDebug() << "Model::deleteJunction, delete junction: " << item->name;
    
    // clear selection
    // otherwise update events will cause net scene to access removed objects
    itemSelectionModel->clearSelection();

    // remove from scene and delete from model
    if ( item->hasPath ) {
        pathit = item->graphicItem1;
        beginRemoveRows(pathit->model->index(item), item->row(), item->row());
        netScene->removeItem(pathit);
        item->parent()->removeChild(item);
        endRemoveRows();
    } 
    if (item->hasPoint) {
        pointit = item->graphicItem2;
        beginRemoveRows(pointit->model->index(item), item->row(), item->row());
        netScene->removeItem(pointit);
        item->parent()->removeChild(item);
        endRemoveRows();
    }
    
    // remove from XML structure
    // TODO
    
}

void Model::deleteConnection(Item *item)
{
    int nodeIndex = item->xmlNode;
    PathElement *pathit;
    PointElement *pointit;

    qDebug() << "Model::deleteConnection, name: " << item->name;
    
    // clear selection
    // otherwise update events will cause net scene to access removed objects
    itemSelectionModel->clearSelection();
    
    // remove from scene and delete from model
    if ( item->hasPath ) {
        pathit = item->graphicItem1;
        beginRemoveRows(pathit->model->index(item), item->row(), item->row());
        netScene->removeItem(pathit);
        item->parent()->removeChild(item);
        endRemoveRows();
    } else if (item->hasPoint) {
        pointit = item->graphicItem2;
        beginRemoveRows(pointit->model->index(item), item->row(), item->row());
        netScene->removeItem(pointit);
        item->parent()->removeChild(item);
        endRemoveRows();
    } else {
        QMessageBox::information(NULL, "Model", "No graphic object was connected to this element!");
        return;
    }

    // remove from XML structure
    // TODO: nodeIndex is not refreshed when nodes are removed
    deleteElement(nodeIndex);
}