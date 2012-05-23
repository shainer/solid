#include <solid/partitioner/volumetree.h>
#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{

using namespace Devices;

VolumeTreeItemPrivate::VolumeTreeItemPrivate(DeviceModified* v, QList< VolumeTreeItem *> c, VolumeTreeItem* p)
    : volume(v)
    , children(c)
    , parent(p)
{}

VolumeTreeItemPrivate::VolumeTreeItemPrivate(const VolumeTreeItemPrivate& other)
    : QSharedData(other)
    , volume(other.volume)
    , children(other.children)
    , parent(other.parent)
{}

VolumeTreeItemPrivate::~VolumeTreeItemPrivate()
{}
    
VolumeTreeItem::VolumeTreeItem(DeviceModified* volume, QList< VolumeTreeItem *> children, VolumeTreeItem* parent)
    : d( new VolumeTreeItemPrivate(volume, children, parent) )
{}

VolumeTreeItem::~VolumeTreeItem()
{}

VolumeTreeItem::VolumeTreeItem(const VolumeTreeItem& other)
    : d( other.d )
{}

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

VolumeTreePrivate::VolumeTreePrivate(VolumeTreeItem *r)
    : root(r)
{}

VolumeTreePrivate::VolumeTreePrivate(const VolumeTreePrivate &other)
    : QSharedData(other)
    , root(other.root)
{}

VolumeTreePrivate::~VolumeTreePrivate()
{}

void VolumeTree::addNode(const QString& parentUdi, DeviceModified* child, VolumeTreeItem* r)
{
    if (r->volume()->name() == parentUdi) {
        r->addChild(child);
        return;
    }
    
    foreach (VolumeTreeItem* c, r->children()) {
        addNode(parentUdi, child, c);
    }
}

void VolumeTreePrivate::print(VolumeTreeItem* r) const
{
    qDebug() << r->volume()->name() << "parent" << (r->parent() ? r->parent()->volume()->name() : "nessuno");
    
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
        StorageVolumeModified* part = dynamic_cast<StorageVolumeModified *>(child->volume());
        
        if (part->partitionType() == StorageVolumeModified::Extended) {
            return child;
        }
    }
    
    return NULL;
}

void VolumeTree::addNode(const QString& parentUdi, DeviceModified* child)
{
    addNode(parentUdi, child, d->root);
}

void VolumeTree::print() const
{
    d->print(d->root);
}

}
}
