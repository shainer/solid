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

#include <solid/partitioner/volumetreemap.h>
#include <solid/block.h>
#include <solid/device.h>
#include <solid/partitioner/utils/utils.h>
#include <solid/partitioner/devices/partition.h>
#include <solid/predicate.h>
#include <ifaces/devicemanager.h>
#include <backends/udisks/udisksmanager.h>

namespace Solid
{
namespace Partitioner
{
    
using namespace Devices;
    
class VolumeTreeMap::Private
{
public:
    Private()
        : backend( new Backends::UDisks::UDisksManager(0) )
    {}
    
    ~Private()
    {}
    
    QMap<QString, VolumeTree> devices;
    QMap<QString, VolumeTree> beginningCopies;
    
    Ifaces::DeviceManager* backend;

    Disk* addDisk(StorageDrive *, const QString &, QMap<QString, VolumeTree> &);
    void detectPartitionsOfDisk(const QString &, QMap<QString, VolumeTree> &);
    void detectFreeSpaceOfDisk(const QString &, QMap<QString, VolumeTree> &);
    
    /* The following functions help detecting free space blocks between partitions */
    QList< FreeSpace* > freeSpaceOfDisk(const VolumeTree &);
    QList< FreeSpace* > findSpace(QList< VolumeTreeItem* >, DeviceModified *);
    FreeSpace* spaceBetweenPartitions(Partition *, Partition *, DeviceModified *);
};

VolumeTreeMap::VolumeTreeMap()
    : d( new Private )
{
    QObject::connect(d->backend, SIGNAL(deviceAdded(QString)), SLOT(doDeviceAdded(QString)));
    QObject::connect(d->backend, SIGNAL(deviceRemoved(QString)), SLOT(doDeviceRemoved(QString)));
}

/* FIXME */
VolumeTreeMap::VolumeTreeMap(const VolumeTreeMap& other)
    : QObject()
    , d( new Private )
{
    d->devices = other.d->devices;
}

VolumeTreeMap::~VolumeTreeMap()
{
    clear();
    d->backend->deleteLater();
    
    delete d;
}

void VolumeTreeMap::build()
{
    clear();
    /*
     * Detection of drives.
     * A new tree is created for each disk on the system.
     */
    foreach(Device dev, Device::listFromType(DeviceInterface::StorageDrive)) {
        QString udi = dev.udi();
        Solid::StorageDrive *drive = dev.as<Solid::StorageDrive>();
        Solid::Block* block = dev.as<Solid::Block>();
        
        if (drive->driveType() == StorageDrive::HardDisk)
        {
            Disk* newDiskCopy = d->addDisk(drive, udi, d->beginningCopies);
            Disk* newDisk = d->addDisk(drive, udi, d->devices);
            
            /*
             * Loop partitions, identified by a particular major number, aren't considered for partitioning.
             * The same applies to disks without a partition table.
             */
            if (block->deviceMajor() != LOOPDEVICE_MAJOR && newDisk->partitionTableScheme() != Utils::NoneScheme) {
                d->detectPartitionsOfDisk(udi, d->beginningCopies);
                d->detectPartitionsOfDisk(udi, d->devices);
                
                d->detectFreeSpaceOfDisk(udi, d->beginningCopies);
                d->detectFreeSpaceOfDisk(udi, d->devices);
            }
        }
    }
}

void VolumeTreeMap::backToOriginal()
{
    d->devices.clear();
    
    qDebug() << "beginning copy";
    d->beginningCopies["/org/freedesktop/UDisks/devices/sda"].print();
    
    d->devices = d->beginningCopies;
    d->devices.detach();
    
    qDebug() << "dopo copia";
    d->devices["/org/freedesktop/UDisks/devices/sda"].print();
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

void VolumeTreeMap::remove(const QString& diskName)
{
    d->devices.remove(diskName);
}

void VolumeTreeMap::clear()
{
    d->devices.clear();
}

/*
 * TODO: implement correctly.
 */
void VolumeTreeMap::doDeviceAdded(QString udi)
{
    Q_UNUSED(udi)
}

void VolumeTreeMap::doDeviceRemoved(QString udi)
{
    Q_UNUSED(udi)
}

Disk* VolumeTreeMap::Private::addDisk(StorageDrive* drive, const QString& udi, QMap<QString, VolumeTree>& devList)
{
    Devices::Disk* disk = new Devices::Disk( drive );
    disk->setName(udi);

    VolumeTree tree( disk );
    devList.insert(disk->name(), tree);

    return disk;
}

void VolumeTreeMap::Private::detectPartitionsOfDisk(const QString& parentUdi, QMap<QString, VolumeTree>& devList)
{
    QList<Device> devs = Device::listFromType(DeviceInterface::StorageVolume, parentUdi);
    Devices::Partition* extended = 0;
    VolumeTree tree = devList[parentUdi];
    
    /*
     * Every detection removes the previous one. This is useful when the detection is repeated
     * after the delivery of a deviceAdded/deviceRemoved signal.
     */
    tree.d->removeAllOfType(DeviceModified::PartitionDevice);
    
    /*
     * For now doesn't consider all the volume types not supported.
     * TODO: erase volumes when isIgnored() == true.
     */
    for (QList<Device>::iterator it = devs.begin(); it != devs.end(); it++) {
        StorageVolume* volume = it->as<StorageVolume>();
        
        if (volume->usage() > StorageVolume::FileSystem) {
            devs.erase(it);
        }
    }
    
    /*
     * Finds the extended partition, if any. This must be done separately because nobody assures us
     * that the extended partition comes before the logicals in the devices list.
     */
    foreach (Device device, devs) {
        StorageVolume* volume = device.as<StorageVolume>();

        if (volume->partitionType() == EXTENDED_TYPE_STRING) {
            extended = new Partition(device);
            extended->setPartitionType(Utils::ExtendedPartition);
            extended->setName(device.udi());
            extended->setParentName(parentUdi);
            
            tree.d->addDevice(parentUdi, extended);
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
        
        part->setName(device.udi());
        part->setParentName(parentName);
        
        tree.d->addDevice(parentName, part);
    }    
}

void VolumeTreeMap::Private::detectFreeSpaceOfDisk(const QString& parentUdi, QMap<QString, VolumeTree>& devList)
{
    VolumeTree tree = devList[parentUdi];
    tree.d->removeAllOfType(DeviceModified::FreeSpaceDevice);
    
    foreach (FreeSpace* space, freeSpaceOfDisk(tree)) {
        tree.d->addDevice(space->parentName(), space);
    }
}

QList< FreeSpace* > VolumeTreeMap::Private::freeSpaceOfDisk(const VolumeTree& diskTree)
{
    QList< VolumeTreeItem* > primaryPartitions = diskTree.rootNode()->children();
    Disk* disk = dynamic_cast< Disk* >(diskTree.root());
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
        Partition* volume1 = NULL;
        Partition* volume2 = NULL;
        FreeSpace *space = NULL;
        
        if (i == -1) {
            volume2 = dynamic_cast< Partition* >(partitions[i+1]->volume());
        }
        else if (i < partitions.size() - 1) {
            volume1 = dynamic_cast< Partition* >(partitions[i]->volume());
            volume2 = dynamic_cast< Partition* >(partitions[i+1]->volume());
        }
        else {
            volume1 = dynamic_cast< Partition* >(partitions[i]->volume());
        }
        
        space = spaceBetweenPartitions(volume1, volume2, parent);
        
        if (space) {
            freeSpaces.append(space);
        }
    }
    
    return freeSpaces;
}

FreeSpace* VolumeTreeMap::Private::spaceBetweenPartitions(Partition* partition1,
                                                          Partition* partition2,
                                                          DeviceModified* parent)
{
    FreeSpace* sp = 0;
    
    /* The logical partition are separated by some bytes used for the Extended Boot Record */
    int spaceBetween = (parent->deviceType() == DeviceModified::PartitionDevice ? SPACE_BETWEEN_LOGICALS : 0);
    
    if (!partition1) {
        qulonglong initialOffset = parent->offset() + spaceBetween;
        if (partition2->offset() > initialOffset) {
            sp = new FreeSpace(initialOffset, partition2->offset() - initialOffset, parent->name());
        }
    }
    else if (!partition2) {
        if (partition1->rightBoundary() + spaceBetween < parent->rightBoundary()) {
            sp = new FreeSpace(partition1->rightBoundary(), parent->rightBoundary() - partition1->rightBoundary() - spaceBetween, parent->name());
        }
    }
    else {
        if (partition1->rightBoundary() + spaceBetween < partition2->offset()) {
            sp = new FreeSpace(partition1->rightBoundary(), (partition2->offset() - partition1->rightBoundary() - spaceBetween), parent->name());
        }
    }
    
    return sp;
}

}
}