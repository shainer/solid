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

#include <solid/partitioner/volumetreemap_p.h>
#include <solid/block.h>
#include <solid/device.h>
#include <solid/partitioner/utils/utils.h>
#include <solid/partitioner/devices/partition.h>
#include <solid/predicate.h>
#include <ifaces/devicemanager.h>

namespace Solid
{
namespace Partitioner
{
    
using namespace Devices;

VolumeTreeMap::VolumeTreeMap()
    : d( new Private(this) )
{}

VolumeTreeMap::VolumeTreeMap(const VolumeTreeMap& other)
    : d( new Private(this) )
{
    d->devices = other.d->devices;
    d->beginningCopies = other.d->beginningCopies;
}

VolumeTreeMap::~VolumeTreeMap()
{
    delete d;
}

QMap< QString, VolumeTree > VolumeTreeMap::deviceTrees() const
{
    return d->devices;
}

VolumeTree VolumeTreeMap::operator[](const QString& diskName) const
{
    return d->devices[diskName];
}

bool VolumeTreeMap::contains(const QString& diskName) const
{
    return d->devices.contains(diskName);
}

QPair< VolumeTree, DeviceModified* > VolumeTreeMap::searchTreeWithDevice(const QString& devName) const
{
    QPair< VolumeTree, DeviceModified* > pair;
    
    foreach (VolumeTree tree, d->devices.values()) {
        DeviceModified* dev = tree.searchDevice(devName);
        
        if (dev) {
            pair = qMakePair< VolumeTree, DeviceModified* >(tree, dev);
            break;
        }
    }
    
    return pair;
}

QPair< VolumeTree, Partition* > VolumeTreeMap::searchTreeWithPartition(const QString& partitionName) const
{
    QPair< VolumeTree, Partition* > pair;
    
    foreach (VolumeTree tree, d->devices.values()) {
        DeviceModified* dev = tree.searchDevice(partitionName);
        
        if (dev && dev->deviceType() == DeviceModified::PartitionDevice) {
            Partition* p = dynamic_cast< Partition* >(dev);
            pair = qMakePair< VolumeTree, Partition* >(tree, p);
            break;
        }
    }
    
    return pair;
}

DeviceModified* VolumeTreeMap::searchDevice(const QString& udi) const
{
    foreach (const VolumeTree& tree, d->devices.values()) {
        DeviceModified* dev = tree.searchDevice(udi);
        
        if (dev) {
            return dev;
        }
    }
    
    return NULL;
}

Partition* VolumeTreeMap::searchPartition(const QString& udi) const
{
    DeviceModified* device = searchDevice(udi);
    
    if (!device || device->deviceType() != DeviceModified::PartitionDevice) {
        return NULL;
    }
    
    return dynamic_cast< Partition* >(device);
}

/*
 * For a disk, adds a new tree.
 * For a partition, repeats all the detection in the correspondent disk, because the layout change can affect free
 * space blocks too.
 */
void VolumeTreeMap::Private::addDevice(QString udi)
{
    Device newDevice(udi);
        
    if (newDevice.is<StorageDrive>()) {
        buildDisk(newDevice);
        synchronize();
    }
    else if (newDevice.is<StorageVolume>()) {
        QString diskName = newDevice.parentUdi();
        detectChildrenOfDisk(diskName);
        synchronize();
    }
}

void VolumeTreeMap::Private::removeDevice(QString udi)
{
    QPair< VolumeTree, DeviceModified* > pair = q->searchTreeWithDevice(udi);
    VolumeTree tree = pair.first;
    DeviceModified* toRemove = pair.second;
        
    if (toRemove) {
        if (toRemove->deviceType() == DeviceModified::DiskDevice) {
            devices.remove(udi);
            beginningCopies.remove(udi);
        }
        else {
            QString diskName = tree.disk()->name();
            detectChildrenOfDisk(diskName);
            synchronize();
        }
    }    
}

VolumeTreeMap::Private::Private(VolumeTreeMap *q_ptr)
    : q(q_ptr)
{}

VolumeTreeMap::Private::~Private()
{
    clear();
}

void VolumeTreeMap::Private::backToOriginal()
{
    devices.clear();
    
    foreach (const QString& disk, beginningCopies.keys()) {
        devices.insert(disk, beginningCopies[disk].copy());
    }
}

void VolumeTreeMap::Private::clear()
{
    devices.clear();
    beginningCopies.clear();
}

void VolumeTreeMap::Private::synchronize()
{
    beginningCopies.clear();
    
    foreach (const QString& disk, devices.keys()) {
        beginningCopies.insert(disk, devices[disk].copy());
    }
}

void VolumeTreeMap::Private::build()
{
    clear();
    
    /*
     * Detection of drives.
     * A new tree is built for each disk on the system.
     */
    foreach(Device dev, Device::listFromType(DeviceInterface::StorageDrive)) {
        buildDisk(dev);
    }
    
    synchronize();
}

void VolumeTreeMap::Private::buildDisk(Device& dev)
{
    QString udi = dev.udi();
    Solid::StorageDrive *drive = dev.as<Solid::StorageDrive>();
    Solid::Block* block = dev.as<Solid::Block>();
    
    if (drive->driveType() == StorageDrive::HardDisk)
    {
        Disk* newDisk = addDisk(dev);
        
       /*
        * Loop partitions, identified by a particular major number, aren't considered for partition detection.
        * The same applies to disks without a partition table.
        */
        if (block->deviceMajor() != LOOPDEVICE_MAJOR && !newDisk->partitionTableScheme().isEmpty()) {
            detectChildrenOfDisk(udi);
        }
    }
}

Disk* VolumeTreeMap::Private::addDisk(Device& device)
{
    Devices::Disk* disk = new Devices::Disk( device );
    VolumeTree tree( disk );
    devices.insert(disk->name(), tree);

    return disk;
}

void VolumeTreeMap::Private::detectChildrenOfDisk(const QString& diskName)
{    
    /*
     * The old detection is removed in both trees. This is useful when repeating the detection after a deviceAdded or
     * deviceRemoved signal has been delivered, and it's harmless in the initial building.
     */
        
    VolumeTree tree = devices[diskName];
    if (tree.valid()) {
        tree.d->removeAllOfType(DeviceModified::PartitionDevice);
        tree.d->removeAllOfType(DeviceModified::FreeSpaceDevice);
    }

    detectPartitionsOfDisk(diskName);
    detectFreeSpaceOfDisk(diskName);
}

void VolumeTreeMap::Private::detectPartitionsOfDisk(const QString& parentUdi)
{
    QList<Device> devs = Device::listFromType(DeviceInterface::StorageVolume, parentUdi);
    Devices::Partition* extended = 0;
    VolumeTree tree = devices[parentUdi];
    
    /* For now doesn't consider all the volume types not supported. */
    for (QList<Device>::iterator it = devs.begin(); it != devs.end(); it++) {
        StorageVolume* volume = it->as<StorageVolume>();
        
        if (volume->usage() > StorageVolume::FileSystem) {
            devs.erase(it);
        }
    }
    
    /*
     * Finds the extended partition, if any. This must be done separately because nobody ensures us
     * that the extended partition comes before the logicals in the devices list.
     */
    foreach (Device device, devs) {
        StorageVolume* volume = device.as<StorageVolume>();

        if (volume->partitionType() == EXTENDED_TYPE_STRING) {
            extended = new Partition(device);
            extended->setPartitionType(Utils::ExtendedPartition);
            
            tree.d->addDevice(extended);
        }
    }
    
    /*
     * Now detects whether the other partitions are primary or logical. Udisks doesn't supply this information, so we
     * simply check if a partition falls inside the boundaries of the extended (if present).
     */
    foreach (Device device, devs) {
        StorageVolume* volume = device.as<StorageVolume>();
        QString parentName = parentUdi;
        
        if (volume->partitionType() == EXTENDED_TYPE_STRING) {
            continue;
        }
        
        Partition* part = new Partition(device);
        
        if (extended && (part->offset() >= extended->offset() && part->rightBoundary() <= extended->rightBoundary())) {
            part->setPartitionType(Utils::LogicalPartition);
            parentName = extended->name();
        } else {
            part->setPartitionType(Utils::PrimaryPartition);
        }
        
        part->setParentName(parentName);
        tree.d->addDevice(part);
    }    
}

void VolumeTreeMap::Private::detectFreeSpaceOfDisk(const QString& parentUdi)
{
    VolumeTree tree = devices[parentUdi];
    
    foreach (FreeSpace* space, freeSpaceOfDisk(tree)) {
        tree.d->addDevice(space);
    }
}

QList< FreeSpace* > VolumeTreeMap::Private::freeSpaceOfDisk(const VolumeTree& diskTree)
{
    QList< VolumeTreeItem* > primaryPartitions = diskTree.rootNode()->children();
    Disk* disk = diskTree.disk();
    VolumeTreeItem* extended = diskTree.extendedNode();
    
    QList< FreeSpace* > freeSpaces;
    
    freeSpaces.append( findSpace(primaryPartitions, disk) );
    
    if (extended) {
        freeSpaces.append( findSpace(extended->children(), extended->volume()) );
    }
    
    return freeSpaces;
}

QList< FreeSpace* > VolumeTreeMap::Private::findSpace(QList< VolumeTreeItem* > partitions, DeviceModified* parent)
{
    QList< FreeSpace* > freeSpaces;
    
    /* If there is no partition, just add a big block of free space. */
    if (partitions.isEmpty()) {
        freeSpaces.append( new FreeSpace(parent->offset(), parent->size(), parent->name()) );
        return freeSpaces;
    }
    
    /*
     * Checks for space, in order:
     * - between the start of the parent (disk or extended partition) and the first partition.
     * - between two generic partitions.
     * - between the last partition and the end of the parent.
     */
    for (int i = -1; i < partitions.size(); i++) {
        DeviceModified* volume1 = NULL;
        DeviceModified* volume2 = NULL;
        FreeSpace *space = NULL;
        
        if (i == -1) {
            volume2 = partitions[i+1]->volume();
        }
        else if (i < partitions.size() - 1) {
            volume1 = partitions[i]->volume();
            volume2 = partitions[i+1]->volume();
        }
        else {
            volume1 = partitions[i]->volume();
        }
        
        space = spaceBetweenPartitions(volume1, volume2, parent);
        
        if (space) {
            freeSpaces.append(space);
        }
    }
    
    return freeSpaces;
}

FreeSpace* VolumeTreeMap::Private::spaceBetweenPartitions(DeviceModified* partition1,
                                                          DeviceModified* partition2,
                                                          DeviceModified* parent)
{
    FreeSpace* sp = 0;
    
    /* Each logical partition is preceeded by some bytes reserved for the Extended Boot Record entry. */
    int spaceBetween = (parent->deviceType() == DeviceModified::PartitionDevice ? SPACE_BETWEEN_LOGICALS : 0);
    
    if (!partition1) {
        qulonglong initialOffset = parent->offset() + spaceBetween;
        if (partition2->offset() > initialOffset) {
            sp = new FreeSpace(initialOffset,
                               partition2->offset() - initialOffset - spaceBetween,
                               parent->name());
        }
    }
    else if (!partition2) {
        if (partition1->rightBoundary() < parent->rightBoundary()) {
            sp = new FreeSpace(partition1->rightBoundary(),
                               parent->rightBoundary() - partition1->rightBoundary(),
                               parent->name());
        }
    }
    else {
        if (partition1->rightBoundary() + spaceBetween < partition2->offset()) {
            sp = new FreeSpace(partition1->rightBoundary(),
                               (partition2->offset() - partition1->rightBoundary() - spaceBetween),
                               parent->name());
        }
    }
    
    return sp;
}

}
}