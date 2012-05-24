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
    delete d->volume;
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

bool VolumeTreeItem::operator==(const VolumeTreeItem& other) const
{
    return d->volume == other.volume();
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

void VolumeTreePrivate::addDevice(const QString& parentUdi, DeviceModified* child, VolumeTreeItem* r)
{
    if (r->volume()->name() == parentUdi) {
        r->addChild(child);
        return;
    }
    
    foreach (VolumeTreeItem* c, r->children()) {
        addDevice(parentUdi, child, c);
    }
}

void VolumeTreePrivate::print(VolumeTreeItem* r) const
{
    qDebug() << r->volume()->name() << "parent" << (r->parent() ? r->parent()->volume()->name() : "nessuno") << "size" << r->volume()->size();
    
    foreach (VolumeTreeItem *c, r->children()) {
        print(c);
    }
}

void VolumeTreePrivate::destroy(VolumeTreeItem* node)
{
    foreach (VolumeTreeItem* child, node->children()) {
        destroy(child);
    }
    
    delete node;
}

/******* VolumeTree *********/
VolumeTree::VolumeTree(VolumeTreeItem* rootItem)
    : d( new VolumeTreePrivate(rootItem) )
{}

VolumeTree::VolumeTree(DeviceModified* rootDevice)
    : d( new VolumeTreePrivate( new VolumeTreeItem(rootDevice)) )
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

DeviceModified* VolumeTree::searchDevice(const QString& name) const
{
    QList< VolumeTreeItem* > stack;
    stack.push_front(d->root);
    
    while (!stack.isEmpty()) {
        VolumeTreeItem* currentNode = stack.takeFirst();
        
        if (currentNode->volume()->name() == name) {
            return currentNode->volume();
        }
        
        foreach (VolumeTreeItem* child, currentNode->children()) {
            stack.push_front(child);
        }
    }
    
    return NULL;
}

void VolumeTree::addDevice(const QString& parentUdi, DeviceModified* child)
{
    d->addDevice(parentUdi, child, d->root);
}

void VolumeTree::print() const
{
    d->print(d->root);
    qDebug() << "---";
}

void VolumeTree::clear()
{
    d->destroy(d->root);
}

}
}
