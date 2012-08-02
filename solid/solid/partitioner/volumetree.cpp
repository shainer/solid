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
#include <solid/partitioner/devices/freespace.h>
#include <solid/partitioner/devices/disk.h>
#include <solid/partitioner/utils/utils.h>
#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{

using namespace Devices;
using namespace Utils;

/******* VolumeTreeItemPrivate *********/
VolumeTreeItem::Private::Private(DeviceModified* v, QList< VolumeTreeItem *> c, VolumeTreeItem* p)
    : volume(v)
    , children(c)
    , parent(p)
{}

VolumeTreeItem::Private::Private()
{}

VolumeTreeItem::Private::~Private()
{}

/******* VolumeTreeItem *********/
VolumeTreeItem::VolumeTreeItem(DeviceModified* volume, QList< VolumeTreeItem *> children, VolumeTreeItem* parent)
    : d( new Private(volume, children, parent) )
{}

VolumeTreeItem::VolumeTreeItem(const VolumeTreeItem& other)
    : d( new Private )
{
    DeviceModified* otherDev = other.volume();
    d->volume = otherDev->copy();
    d->parent = other.parent();
    
    foreach (VolumeTreeItem* child, other.children()) {
        VolumeTreeItem* copyChild = new VolumeTreeItem(*child);
        copyChild->d->parent = this;
        d->children.append(copyChild);
    }
}

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

void VolumeTreeItem::clearChildren()
{
    qDeleteAll(d->children);
}

bool VolumeTreeItem::operator==(const VolumeTreeItem& other) const
{
    return (d->volume == other.volume());
}

/******* VolumeTreePrivate *********/
VolumeTreePrivate::VolumeTreePrivate(VolumeTreeItem *r)
    : root(r)
    , valid(true)
{}

VolumeTreePrivate::VolumeTreePrivate(const VolumeTreePrivate &other)
    : QSharedData(other)
    , root(other.root)
    , valid(other.valid)
{}

VolumeTreePrivate::VolumeTreePrivate()
    : valid(false)
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

void VolumeTreePrivate::addDevice(DeviceModified* child)
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        
        if (currentNode->volume()->name() == child->parentName()) {
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

void VolumeTreePrivate::removeAllOfType(DeviceModified::DeviceModifiedType type)
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        
        if (currentNode->volume()->deviceType() == type) {
            VolumeTreeItem* parent = currentNode->parent();
                
            if (parent) {
                parent->removeChild(currentNode);
            }
            
            delete currentNode;
            continue;
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

VolumeTreeItem* VolumeTreePrivate::leftNode(DeviceModified* dev)
{
    VolumeTreeItem* node = searchNode( dev->name() );
    Q_ASSERT(node);
    
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

VolumeTreeItem* VolumeTreePrivate::rightNode(DeviceModified* dev)
{
    VolumeTreeItem* node = searchNode( dev->name() );
    Q_ASSERT(node);
    
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

VolumeTreeItem* VolumeTreePrivate::searchContainer(qulonglong offset, qulonglong size)
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        DeviceModified* currentDevice = currentNode->volume();
       
        if (currentDevice->deviceType() == DeviceModified::FreeSpaceDevice) {
            FreeSpace* space = dynamic_cast< FreeSpace* >(currentDevice);

            if (space->offset() <= offset && space->rightBoundary() >= (offset + size)) {
                return currentNode;
            }
        }
        
        foreach (VolumeTreeItem* child, currentNode->children()) {
            stack.push_front(child);
        }
    }
    
    return NULL;
}

bool VolumeTreePrivate::splitCreationContainer(qulonglong offset, qulonglong size)
{
    VolumeTreeItem* containerNode = searchContainer(offset, size);
    
    if (!containerNode) {
        return false;
    }
    
    DeviceModified* container = containerNode->volume();
    qulonglong containerOffset = container->offset();
    qulonglong containerSize = container->size();
    
    if (offset > containerOffset) {
        qulonglong leftSize = offset - containerOffset;
        
        FreeSpace* leftSpace = new FreeSpace(containerOffset, leftSize, container->parentName());
        addDevice(leftSpace);
    }
    
    if (containerSize > size) {
        qulonglong rightOffset = (offset + size);
        qulonglong rightSize = container->rightBoundary() - (offset + size);
        
        FreeSpace* rightSpace = new FreeSpace(rightOffset, rightSize, container->parentName());
        addDevice(rightSpace);
    }
    
    delete containerNode;
    return true;
}

void VolumeTreePrivate::mergeAndDelete(const QString& partitionName)
{
    VolumeTreeItem* partitionNode = searchNode(partitionName);
    Q_ASSERT(partitionNode);
    
    VolumeTreeItem* left = leftNode(partitionNode->volume());
    VolumeTreeItem* right = rightNode(partitionNode->volume());
    DeviceModified* leftSibling = 0;
    DeviceModified* rightSibling = 0;
    
    if (left) {
        leftSibling = left->volume();
    }
    
    if (right) {
        rightSibling = right->volume();
    }
    
    qulonglong size = partitionNode->volume()->size();
    qulonglong offset = partitionNode->volume()->offset();
    DeviceModified* parent = partitionNode->parent()->volume();
    qulonglong spaceBetween = (parent->deviceType() == DeviceModified::PartitionDevice) ? SPACE_BETWEEN_LOGICALS : 0;
    
    delete partitionNode;
    
    /*
     * If there's free space to the left and/or right of the deleted partition, merge
     * all the free space in one single block.
     * 
     * All free space blocks before a logical partition don't consider the EBR (which always comes before the associated
     * logical partition) as part of their size. With this deletion there isn't the EBR anymore so we consider it again.
     */
    if (leftSibling && leftSibling->deviceType() == DeviceModified::FreeSpaceDevice) {
        offset = leftSibling->offset();
        size += leftSibling->size() + spaceBetween;
        
        delete left;
    } else if (leftSibling) { /* same here */
        offset -= spaceBetween;
        size += spaceBetween;
    }
    
    if (rightSibling && rightSibling->deviceType() == DeviceModified::FreeSpaceDevice) {
        size += rightSibling->size();
        delete right;
    }
    
    FreeSpace* newSpace = new FreeSpace(offset, size, parent->name());
    addDevice(newSpace);
}

void VolumeTreePrivate::destroy(VolumeTreeItem* node)
{
    foreach (VolumeTreeItem* child, node->children()) {
        destroy(child);
    }
    
    delete node;
}

/* DEBUG METHOD */
void VolumeTreePrivate::print(VolumeTreeItem* r) const
{
    QTextStream out(stdout);
    out << "-- " << r->volume()->description() << " PARENT " << (r->parent() ? r->parent()->volume()->name() : " nessuno ") << " OFFSET " << r->volume()->offset() << " SIZE " << r->volume()->size();
    
    if (r->volume()->deviceType() == DeviceModified::PartitionDevice) {
        Partition* p = dynamic_cast< Partition* >(r->volume());
        out << " -- " << p->filesystem().name();
    }
    
    out << endl;
    
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
    : d( new VolumeTreePrivate )
{}

VolumeTree::VolumeTree(const VolumeTree& other)
    : d( other.d )
{}

VolumeTree::~VolumeTree()
{}

bool VolumeTree::valid() const
{
    return d->valid;
}

Disk* VolumeTree::disk() const
{
    Q_ASSERT(d->valid);
    Q_ASSERT(d->root);
    
    return dynamic_cast< Disk* >(d->root->volume());
}

VolumeTreeItem* VolumeTree::rootNode() const
{
    return d->root;
}

VolumeTree VolumeTree::copy() const
{
    Q_ASSERT(d->valid);
    Q_ASSERT(d->root);
    
    VolumeTreeItem* copyRootItem = new VolumeTreeItem(*(d->root));
    VolumeTree copy(copyRootItem);
    return copy;
}

VolumeTreeItem* VolumeTree::extendedNode() const
{
    Q_ASSERT(d->valid);
    
    foreach (VolumeTreeItem* child, d->root->children()) {
        Partition* part = dynamic_cast< Partition* >(child->volume());
        
        if (part && part->partitionType() == ExtendedPartition) {
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

DeviceModified* VolumeTree::leftDevice(DeviceModified* dev)
{
    VolumeTreeItem* left = d->leftNode(dev);
    
    if (!left) {
        return NULL;
    }
    
    return left->volume();
}

DeviceModified* VolumeTree::rightDevice(DeviceModified* dev)
{
    VolumeTreeItem* right = d->rightNode(dev);
    
    if (!right) {
        return NULL;
    }
    
    return right->volume();
}

QList< Partition* > VolumeTree::partitions() const
{
    Q_ASSERT(d->valid);
    
    QList< Partition* > devices;
    
    foreach (VolumeTreeItem* item, d->root->children()) {
        DeviceModified* device = item->volume();
        
        if (device->deviceType() == DeviceModified::PartitionDevice) {
            devices.append( dynamic_cast< Partition* >(device) );
        }
    }
    
    return devices;
}

QList< Partition* > VolumeTree::logicalPartitions() const
{
    Q_ASSERT(d->valid);
    QList< Partition *> logicals;
    VolumeTreeItem* ex = extendedNode();   

    if (ex) {
        foreach (VolumeTreeItem* item, ex->children()) {
            DeviceModified* device = item->volume();
            
            if (device->deviceType() == DeviceModified::PartitionDevice) {
                logicals.append( dynamic_cast< Partition* >(device) );
            }
        }
    }
        
    return logicals;
}

QList< FreeSpace* > VolumeTree::freeSpaceBlocks(const QString& parentName) const
{
    Q_ASSERT(d->valid);
    QList< FreeSpace* > blocks;
    VolumeTreeItem* parent = searchNode(parentName);
    
    if (parent) {
        foreach (VolumeTreeItem* item, parent->children()) {
            DeviceModified* device = item->volume();
            
            if (device->deviceType() == DeviceModified::FreeSpaceDevice) {
                blocks.append( dynamic_cast< FreeSpace* >(device) );
            }
        }
    }
    
    return blocks;
}

QList< DeviceModified* > VolumeTree::allDevices(bool addBelowMin) const
{
    Q_ASSERT(d->valid);
    
    Disk* diskDev = disk();
    QList< DeviceModified* > devices;
    QList< VolumeTreeItem* > stack;
    stack.push_front(d->root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        DeviceModified* currentDevice = currentNode->volume();
        
        /* When requested, skip free space blocks too small for accomodating a partition */
        if (!addBelowMin &&
            currentDevice->deviceType() == DeviceModified::FreeSpaceDevice
            && currentDevice->size() < diskDev->minimumPartitionSize()) {
            continue;
        }
        
        devices.append(currentDevice);
        
        /* This is necessary to prepare a list sorted by initial offset */
        QListIterator< VolumeTreeItem* > it( currentNode->children() );
        for (it.toBack(); it.hasPrevious(); ) {
            stack.push_front( it.previous() );
        }
    }
    
    return devices;
}

VolumeTreeItem* VolumeTree::searchNode(const QString& name) const
{
    Q_ASSERT(d->valid);
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

DeviceModified* VolumeTree::parentDevice(DeviceModified* device) const
{
    Q_ASSERT(d->valid);
    VolumeTreeItem* node = searchNode( device->name() );
    Q_ASSERT(node);
    
    if (node->parent()) {
        return node->parent()->volume();
    }
    
    return NULL;
}

void VolumeTree::print() const
{
    d->print(d->root);
    qDebug() << "---";
}

}
}
