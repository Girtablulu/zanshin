/*
    <one line to give the library's name and an idea of what it does.>
    Copyright (C) 2012  Christian Mollekopf <chrigi_1@fastmail.fm>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "reparentingmodel.h"
#include "todonode.h"
#include "globaldefs.h"
#include "todonodemanager.h"
#include <KLocalizedString>
#include <KIcon>

ReparentingModel::ReparentingModel(ReparentingStrategy* strategy, QObject* parent)
:   TodoProxyModelBase(TodoProxyModelBase::MultiMapping, parent),
    m_strategy(strategy)
{
    m_strategy->setModel(this);
}

ReparentingModel::~ReparentingModel()
{

}

void ReparentingModel::init()
{
    m_strategy->init();
}

TodoNode* ReparentingModel::createInbox() const
{
    return 0;
}

TodoNode *ReparentingModel::createNode(const Id &identifier, const Id &parentIdentifier, const QString &name, const QModelIndex &sourceIndex)
{
    if (m_parentMap.contains(identifier)) { //We already have this node, we only need to update it
        TodoNode *node = reparentNode(identifier, IdList() << parentIdentifier, sourceIndex);
        if (!sourceIndex.isValid() && !name.isEmpty()) {
            node->setData(name, 0, Qt::DisplayRole);
        }
        return node;
    }
    
//     kDebug() << "add node" << name << identifier << parentIdentifier << sourceIndex;
    TodoNode* parentNode = 0;
    if (parentIdentifier >= 0) {
        if (!m_parentMap.contains(parentIdentifier)) {
//             kDebug() << "creating missing parent: " << parentIdentifier;
            createNode(parentIdentifier, -1, "unknown");
        }
        Q_ASSERT(m_parentMap.contains(parentIdentifier));
        parentNode = m_parentMap.value(parentIdentifier);
    }

    int row = 0;
    if (parentNode) {
        row = parentNode->children().size();
    } else {
        row = m_manager->roots().size();
    }
    beginInsertRows(m_manager->indexForNode(parentNode, 0), row, row);
    TodoNode *node;
    if (sourceIndex.isValid()) {
        node = new TodoNode(sourceIndex, parentNode);
        //Don't set anything on real nodes
    } else { //For virtual nodes
        node = new TodoNode(parentNode);
        node->setData(name, 0, Qt::DisplayRole);
        node->setData(name, 0, Qt::EditRole);
        //     m_strategy->setData(node, identifier);
    }

    Q_ASSERT(node);
    m_parentMap[identifier] = node;
    m_manager->insertNode(node);
//     kDebug() << identifier << node;
    endInsertRows();
    Q_ASSERT(!m_parentMap.values().contains(0));
    return node;
}

void ReparentingModel::removeNode(TodoNode *root, bool removeChildren)
{
//                 foreach(TodoNode *childNode, node->children()) {
//                     Id childId = m_parentMap.key(childNode);
// //                     kDebug() << "child " << childId;
//                     IdList parents = m_strategy->getParents(childNode->rowSourceIndex());
//                     parents.removeAll(id); //Don't try to re-add it to the current parent
//                     reparentNode(childId, parents, childNode->rowSourceIndex());
//                 }

    kDebug() << "removeNode " << m_parentMap.key(root);


    if (removeChildren) {
//         foreach (TodoNode *child, root->children()) {
//             removeNode(child, removeChildren);
//         }
        foreach(TodoNode *childNode, root->children()) {
            Id childId = m_parentMap.key(childNode);
            if (m_strategy->reparentOnRemoval(childId)) {
                kDebug() << "child " << childId;
                IdList parents = m_strategy->getParents(childNode->rowSourceIndex(), IdList() << m_parentMap.key(root));//Don't try to re-add it to the current parent (which is not yet removed)
//                 parents.removeAll(m_parentMap.key(childNode->parent())); //Don't try to re-add it to the current parent (which is not yet removed)
                reparentNode(childId, parents, childNode->rowSourceIndex());
            } else {
                removeNode(childNode, true);
            }
        }
    } else {
        foreach(TodoNode *childNode, root->children()) {
            childNode->setParent(0);
        }
    }

    QModelIndex proxyParentIndex = m_manager->indexForNode(root->parent(), 0);
    int row = 0;

    if (root->parent()) {
        row = root->parent()->children().indexOf(root);
    } else {
        row = m_manager->roots().indexOf(root);
    }

    beginRemoveRows(proxyParentIndex, row, row);
    Q_ASSERT(m_parentMap.values().contains(root));
    Id id = m_parentMap.key(root);
    m_strategy->onNodeRemoval(id);
    m_parentMap.remove(id);
    m_manager->removeNode(root);
    delete root;
    endRemoveRows();
}

void ReparentingModel::removeNodeById(Id id)
{
    removeNode(m_parentMap.value(id), true);
}


TodoNode *ReparentingModel::reparentNode(const Id& p, const IdList& parents, const QModelIndex &sourceIndex)
{
    Q_ASSERT(!m_parentMap.values().contains(0));
    if (p < 0) {
        kWarning() << "invalid item";
        return 0;
    }
    kDebug() << p << parents << sourceIndex;
    //TODO handle multiple parents
    Q_ASSERT(m_parentMap.contains(p));
    TodoNode *node = m_parentMap.value(p);
    Q_ASSERT(node);

    //We want to reparent to same parent to update sourceIndex
    if (node->rowSourceIndex() == sourceIndex) {
        //Reparent only if parent has changed
        if ((parents.isEmpty() && !node->parent()) || (!parents.isEmpty() && (node->parent() == m_parentMap.value(parents.first())))) {
            kDebug() << "nothing changed" << node->parent() << m_parentMap.value(parents.first());
            Q_ASSERT(!m_parentMap.values().contains(0));
            return 0;
        }
    }
    
    const QString &name = node->data(0, Qt::DisplayRole).toString();
    QList<TodoNode*> children = node->children();

    //remove node from any current parent
    removeNode(node, false);

    TodoNode *newNode;
    if (parents.isEmpty()) {
        newNode = createNode(p, -1, name, sourceIndex);
    } else {
        newNode = createNode(p, parents.first(), name, sourceIndex);
    }
    
    foreach (TodoNode* child, children) {
        child->setParent(newNode);
    }
    Q_ASSERT(m_parentMap.contains(p));
    Q_ASSERT(!m_parentMap.values().contains(0));
    return newNode;
}

void ReparentingModel::renameNode(const Id& identifier, const QString& name)
{
//     kDebug() << "renamed " << identifier << " to " << name;
    TodoNode *node = m_parentMap[identifier];
    node->setData(name, 0, Qt::DisplayRole);
    node->setData(name, 0, Qt::EditRole);
    const QModelIndex &begin = m_manager->indexForNode(node, 0);
    const QModelIndex &end = m_manager->indexForNode(node, 0);
    emit dataChanged(begin, end);
}

void ReparentingModel::onSourceInsertRows(const QModelIndex& sourceIndex, int begin, int end)
{
//     kDebug() << sourceIndex << begin << end;
    for (int i = begin; i <= end; i++) {
        const QModelIndex &sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        
        if (!sourceChildIndex.isValid()) {
            kWarning() << "invalid sourceIndex";
            continue;
        }

        Id id = m_strategy->getId(sourceChildIndex);
        IdList parents = m_strategy->getParents(sourceChildIndex);
//         if (parents.isEmpty()) {
//             createNode(id, -1);
//         } else {
//             createNode(id, parents.first());
//         }
//         kDebug() << "adding node: " << sourceChildIndex.data(Qt::DisplayRole).toString() << id << parents;
        //The item has already been inserted before, update and reparent
        if (m_parentMap.contains(id)) {
//             kDebug() << "update parent";
            reparentNode(id, parents, sourceChildIndex);
            continue;
        }

        //Insert new item
        if (parents.isEmpty()) {
            m_parentMap[id] = addChildNode(sourceChildIndex, 0);
        } else {
            foreach (const Id &p, parents) {
//                 kDebug() << "added node to parent: " << p;
                TodoNode *parent = m_parentMap.value(p);
                if (!parent) { //if the item is before the parent, the parent may not be existing yet.
                    kDebug() << "creating parent " << p;
                    createNode(p, -1, "unknown");
                    Q_ASSERT(m_parentMap.contains(p));
                    parent = m_parentMap.value(p);
                }
                Q_ASSERT(parent);
                m_parentMap[id] = addChildNode(sourceChildIndex, parent); //TODO set data
                //TODO parent map is a multimap actually, act accordingly (this applies to all places where parent map is used). This because a node can be in multiple places at the same time.
            }
        }

        //Insert children too
        if (sourceModel()->hasChildren(sourceChildIndex)) {
//             kDebug() << id << " has children";
            onSourceInsertRows(sourceChildIndex, 0, sourceModel()->rowCount(sourceChildIndex)-1);
        }
    }

    Q_ASSERT(!m_parentMap.values().contains(0));
}

void ReparentingModel::onSourceDataChanged(const QModelIndex& begin, const QModelIndex& end)
{
    Q_ASSERT(!m_parentMap.values().contains(0));
    kDebug() << begin << end;
    for (int row = begin.row(); row <= end.row(); row++) {
        const QModelIndex &index = sourceModel()->index(row, 0, begin.parent());
        Id id = m_strategy->getId(index);
        const IdList &parents = m_strategy->getParents(index);
//         kDebug() << index.data(Qt::DisplayRole).toString() << id << parents;

        if (m_parentMap.contains(id)) { //In case we get updates for indexes which haven't been inserted yet
            reparentNode(id, parents, index);
        } else {
            kWarning() << id << " is missing";
        }

        const QModelIndexList &list = mapFromSourceAll(index);
        foreach (const QModelIndex &proxyIndex, list) {
            dataChanged(proxyIndex, proxyIndex);
        }
    }
    Q_ASSERT(!m_parentMap.values().contains(0));
}


void ReparentingModel::onSourceRemoveRows(const QModelIndex& sourceIndex, int begin, int end)
{
//     kDebug() << begin << end;
    for (int i = begin; i <= end; ++i) {
        QModelIndex sourceChildIndex = sourceModel()->index(i, 0, sourceIndex);
        if (!sourceChildIndex.isValid()) {
//             kDebug() << "invalid source";
            continue;
        }
        QList<TodoNode*> nodes = m_manager->nodesForSourceIndex(sourceChildIndex);
        foreach (TodoNode *node, nodes) {
            removeNode(node, true);
            
//             Id id = m_parentMap.key(node);
//             kDebug() << "remove node " << id;
// //             if (m_strategy->reparentOnRemoval(id)) {
//                 foreach(TodoNode *childNode, node->children()) {
//                     Id childId = m_parentMap.key(childNode);
//                     if (m_strategy->reparentOnRemoval(childId)) {
//                         kDebug() << "child " << childId;
//                         IdList parents = m_strategy->getParents(childNode->rowSourceIndex());
//                         parents.removeAll(id); //Don't try to re-add it to the current parent (which is not yet removed)
//                         reparentNode(childId, parents, childNode->rowSourceIndex());
//                     }
//                 }
//                 removeNode(node, true);
// //             } else {
// //                 removeNode(node, true);
// //             }
        }
    }
}

void ReparentingModel::resetInternalData()
{
    m_parentMap.clear();
    m_strategy->reset();
    TodoProxyModelBase::resetInternalData();
}


