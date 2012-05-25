/*
    Copyright 2012 Lisa Vitolo <shainer@chakra-project.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/

#include <solid/partitioner/volumetree.h>
#include "devices/freespace.h"
#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{

using namespace Devices;

/******* VolumeTreeItemPrivate *********/
VolumeTreeItem::Private::Private(DeviceModified* v, QList< VolumeTreeItem *> c, VolumeTreeItem* p)
    : volume(v)
    , children(c)
    , parent(p)
{}

VolumeTreeItem::Private::~Private()
{}

/******* VolumeTreeItem *********/
VolumeTreeItem::VolumeTreeItem(DeviceModified* volume, QList< VolumeTreeItem *> children, VolumeTreeItem* parent)
    : d( new Private(volume, children, parent) )
{}

VolumeTreeItem::~VolumeTreeItem()
{
    if (d->parent) {
        d->parent->removeChild(this);
    }
    
    foreach (VolumeTreeItem* child, d->children) {
        delete child;
    }
    
    delete d->volume;
    delete d;
}

DeviceModified* VolumeTreeItem::volume() const
{
    return d->volume;
}

QList<VolumeTreeItem *> VolumeTreeItem::children() const
{
    return d->children;
}

VolumeTreeItem* VolumeTreeItem::parent() const
{
    return d->parent;
}

void VolumeTreeItem::addChild(DeviceModified* child)
{
    VolumeTreeItem* item = new VolumeTreeItem(child, QList<VolumeTreeItem *>(), this);
    bool inserted = false;
        
    for (int i = 0; i < d->children.size(); i++) {
        DeviceModified* device = d->children[i]->volume();
        
        if (device->offset() > child->offset()) {
            d->children.insert(i, item);
            inserted = true;
            break;
        }
    }
    
    if (!inserted) {
        d->children.append(item);
    }
}

void VolumeTreeItem::removeChild(VolumeTreeItem* child)
{
    d->children.removeOne(child);
}

bool VolumeTreeItem::operator==(const VolumeTreeItem& other) const
{
    return (d->volume == other.volume());
}

/******* VolumeTreePrivate *********/
VolumeTreePrivate::VolumeTreePrivate(VolumeTreeItem *r)
    : root(r)
{}

VolumeTreePrivate::VolumeTreePrivate(const VolumeTreePrivate &other)
    : QSharedData(other)
    , root(other.root)
{}

VolumeTreePrivate::~VolumeTreePrivate()
{}

VolumeTreeItem* VolumeTreePrivate::searchNode(const QString& name) const
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        
        if (currentNode->volume()->name() == name) {
            return currentNode;
        }
        
        foreach (VolumeTreeItem* child, currentNode->children()) {
            stack.push_front(child);
        }
    }
    
    return NULL;
}

void VolumeTreePrivate::addDevice(const QString& parentName, DeviceModified* child)
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        
        if (currentNode->volume()->name() == parentName) {
            currentNode->addChild(child);
            return;
        }
        
        foreach (VolumeTreeItem* child, currentNode->children()) {
            stack.push_front(child);
        }
    }
}

void VolumeTreePrivate::removeDevice(const QString& deviceName)
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        
        if (currentNode->volume()->name() == deviceName) {
            VolumeTreeItem* parent = currentNode->parent();
            
            if (parent) {
                parent->removeChild(currentNode);
            }
            
            delete currentNode;
            return;
        }
        
        foreach (VolumeTreeItem* child, currentNode->children()) {
            stack.push_front(child);
        }
    }
}

void VolumeTreePrivate::clear()
{
    delete root;
}

VolumeTreeItem* VolumeTreePrivate::leftSibling(VolumeTreeItem* node)
{
    if (!node->parent()) {
        return NULL;
    }
    
    QList< VolumeTreeItem* > siblings = node->parent()->children();
    int index = siblings.indexOf(node);
    
    if (index > 0) {
        return siblings[index-1];
    }
    
    return NULL;
}

VolumeTreeItem* VolumeTreePrivate::rightSibling(VolumeTreeItem* node)
{
    if (!node->parent()) {
        return NULL;
    }
    
    QList< VolumeTreeItem* > siblings = node->parent()->children();
    int index = siblings.indexOf(node);
    
    if (index < siblings.size() - 1) {
        return siblings[index+1];
    }
    
    return NULL;
}

DeviceModified* VolumeTreePrivate::leftDevice(VolumeTreeItem* node)
{
    VolumeTreeItem* left = leftSibling(node);
    
    if (!left) {
        return NULL;
    }
    
    return left->volume();
}

DeviceModified* VolumeTreePrivate::rightDevice(VolumeTreeItem* node)
{
    VolumeTreeItem* right = rightSibling(node);
    
    if (!right) {
        return NULL;
    }
    
    return right->volume();
}

bool VolumeTreePrivate::splitCreationContainer(qulonglong offset, qulonglong size)
{
    FreeSpace* container = 0;
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        DeviceModified* currentDevice = currentNode->volume();
       
        if (currentDevice->deviceType() == DeviceModified::FreeSpaceDevice) {
            FreeSpace* space = dynamic_cast< FreeSpace* >(currentDevice);
            
            if (space->offset() <= offset && space->size() >= size) {
                container = space;
            }
        }
        
        foreach (VolumeTreeItem* child, currentNode->children()) {
            stack.push_front(child);
        }
    }
    
    if (!container) {
        return false;
    }
    
    FreeSpace* leftSpace = 0;
    FreeSpace* rightSpace = 0;
    qulonglong containerOffset = container->offset();
    qulonglong containerSize = container->size();
    
    if (offset > containerOffset) {
        leftSpace = new FreeSpace(containerOffset, offset - containerOffset, container->parentName());
        addDevice(container->parentName(), leftSpace);
    }
    
    if (containerSize > size) {
        rightSpace = new FreeSpace((offset + size), (container->rightBoundary()) - (offset + size), container->parentName());
        addDevice(container->parentName(), rightSpace);
    }
    
    delete container;
    return true;
}

void VolumeTreePrivate::mergeAndDelete(const QString& partitionName)
{
    VolumeTreeItem* partitionNode = searchNode(partitionName);
    
    VolumeTreeItem* leftNode = leftSibling(partitionNode);
    VolumeTreeItem* rightNode = rightSibling(partitionNode);
    DeviceModified* leftSibling = leftNode->volume();
    DeviceModified* rightSibling = rightNode->volume();
    
    qulonglong size = partitionNode->volume()->size();
    qulonglong offset = partitionNode->volume()->offset();
    QString parentName = partitionNode->volume()->parentName();
    
    delete partitionNode;
    
    /*
     * If there's free space to the left and/or right of the deleted partition, merge
     * all the free space in one single block.
     */
    if (leftSibling && leftSibling->deviceType() == DeviceModified::FreeSpaceDevice) {
        offset = leftSibling->offset();
        size += leftSibling->size();
        delete leftNode;
    }
    
    if (rightSibling && rightSibling->deviceType() == DeviceModified::FreeSpaceDevice) {
        size += rightSibling->size();
        delete rightNode;
    }
    
    FreeSpace* newSpace = new FreeSpace(offset, size, parentName);
    addDevice(parentName, newSpace);
}

void VolumeTreePrivate::destroy(VolumeTreeItem* node)
{
    foreach (VolumeTreeItem* child, node->children()) {
        destroy(child);
    }
    
    delete node;
}


void VolumeTreePrivate::print(VolumeTreeItem* r) const
{
    qDebug() << r->volume()->name() << "parent" << (r->parent() ? r->parent()->volume()->name() : "nessuno") << "offset" << r->volume()->offset() << "size" << r->volume()->size();
    
    foreach (VolumeTreeItem *c, r->children()) {
        print(c);
    }
}

/******* VolumeTree *********/
VolumeTree::VolumeTree(VolumeTreeItem* rootItem)
    : d( new VolumeTreePrivate(rootItem) )
{}

VolumeTree::VolumeTree(DeviceModified* rootDevice)
    : d( new VolumeTreePrivate( new VolumeTreeItem(rootDevice)) )
{}

VolumeTree::VolumeTree()
{}

VolumeTree::VolumeTree(const VolumeTree& other)
    : d( other.d )
{}

VolumeTree::~VolumeTree()
{}

DeviceModified* VolumeTree::root() const
{
    return d->root->volume();
}

VolumeTreeItem* VolumeTree::rootNode() const
{
    return d->root;
}

VolumeTreeItem* VolumeTree::extendedNode() const
{
    foreach (VolumeTreeItem* child, d->root->children()) {
        Partition* part = dynamic_cast<Partition *>(child->volume());
        
        if (part->partitionType() == Extended) {
            return child;
        }
    }
    
    return NULL;
}

DeviceModified* VolumeTree::extendedPartition() const
{
    VolumeTreeItem* ex = extendedNode();
    if (!ex) {
        return NULL;
    }
    
    return ex->volume();
}

QList< DeviceModified* > VolumeTree::partitions(bool free) const
{
    QList< DeviceModified* > devices;
    
    foreach (VolumeTreeItem* item, d->root->children()) {
        DeviceModified* device = item->volume();
        
        if (!free && device->deviceType() == DeviceModified::FreeSpaceDevice) {
            continue;
        }
        
        devices.append(device);
    }
    
    return devices;
}

QList< DeviceModified* > VolumeTree::logicalPartitions(bool free) const
{
    QList< DeviceModified *> logicals;
    VolumeTreeItem* ex = extendedNode();
    

    if (ex) {
        foreach (VolumeTreeItem* item, ex->children()) {
            DeviceModified* device = item->volume();
            
            if (!free && device->deviceType() == DeviceModified::FreeSpaceDevice) {
                continue;
            }
            
            logicals.append(device);
        }
    }
        
    return logicals;
}

VolumeTreeItem* VolumeTree::searchNode(const QString& name) const
{
    return d->searchNode(name);
}

DeviceModified* VolumeTree::searchDevice(const QString& name) const
{
    VolumeTreeItem* node = searchNode(name);
    
    if (!node) {
        return NULL;
    }
    
    return node->volume();
}

void VolumeTree::print() const
{
    d->print(d->root);
    qDebug() << "---";
}

}
}
