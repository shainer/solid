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
#include <solid/partitioner/volumemanager.h>
#include <solid/partitioner/devices/partition.h>
#include <solid/partitioner/devices/disk.h>
#include <solid/partitioner/actions/formatpartitionaction.h>
#include <solid/partitioner/actions/action.h>
#include "actions/createpartitionaction.h"
#include "actions/removepartitionaction.h"
#include "actions/resizepartitionaction.h"
#include <solid/device.h>
#include <kglobal.h>

namespace Solid
{   
namespace Partitioner
{

using namespace Devices;
    
class VolumeManagerHelper
{
public:
    VolumeManagerHelper()
        : q(0)
    {}
    
    virtual ~VolumeManagerHelper()
    {
        delete q;
    }
    
    VolumeManager* q;
};

K_GLOBAL_STATIC(VolumeManagerHelper, s_volumemanager);

VolumeManager::VolumeManager()
    : executer(0)
{
    Q_ASSERT(!s_volumemanager->q);
    s_volumemanager->q = this;
    
    detectDevices();
}

VolumeManager::~VolumeManager()
{}

VolumeManager* VolumeManager::instance()
{
    if (!s_volumemanager->q) {
        new VolumeManager;
    }
    
    return s_volumemanager->q;
}

void VolumeManager::detectDevices()
{
    /*
     * Detection of drives.
     * A new tree is created for each disk on the system.
     */
    foreach(Device dev, Device::listFromType(DeviceInterface::StorageDrive)) {
        QString udi = dev.udi();
        Solid::StorageDrive *drive = dev.as<Solid::StorageDrive>();
        
        if (drive->driveType() == StorageDrive::HardDisk)
        {
            Devices::Disk* driveModified = new Devices::Disk( drive );
            driveModified->setName(udi);
            
            VolumeTree tree( driveModified );
            volumeTrees.insert(driveModified->name(), tree);
        }
    }
    
    /*
     * Detection of partitions.
     * Detection of partition type (primary, logical or extended) is done separately because udisks doesn't provide
     * this specific information.
     * 
     * Each partition is a child of the disk it belongs to, except logical partitions; those are children of the
     * correspondent extended partition.
     */
    QMultiMap<QString, Devices::Partition *> partitions;
    foreach(Device dev, Device::listFromType(DeviceInterface::StorageVolume)) {
        StorageVolume* volume = dev.as<StorageVolume>();
        
        Devices::Partition* volumeModified = new Devices::Partition(volume);
        volumeModified->setName(dev.udi());
        volumeModified->setParentName(dev.parentUdi());
        
        if (!volumeModified->name().contains("loop")) {
            partitions.insertMulti(volumeModified->parentName(), volumeModified);
        }
    }
    
    foreach (const QString& parent, partitions.uniqueKeys()) {
        Devices::Partition* extended = NULL;
        VolumeTree tree = volumeTrees.value(parent);
        
        foreach (Devices::Partition* volume, partitions.values(parent)) {
            if (volume->uuid().isEmpty()) {
                volume->setPartitionType(Extended);
                
                tree.addDevice(volume->parentName(), volume);
                extended = volume;
            }
        }
        
        foreach (Devices::Partition* volume, partitions.values(parent)) {
            QString parentName = parent;
            
            if (!volumeTrees.contains(parentName)) {
                continue;
            }
            
            VolumeTree tree = volumeTrees.value(parentName);
            
            if (volume->partitionType() == Extended) {
                continue;
            }
            else if (extended && (volume->offset() >= extended->offset() && volume->rightBoundary() <= extended->rightBoundary())) {
                volume->setPartitionType(Logical);
                parentName = extended->name();
            }
            else {
                volume->setPartitionType(Primary);
            }
            
            tree.addDevice(parentName, volume);
        }
    }
    
    /*
     * Detection of blocks of free space.
     */
    foreach (VolumeTree disk, volumeTrees.values()) {
        foreach (Devices::FreeSpace* space, Device::freeSpaceOfDisk(disk)) {
            disk.addDevice(space->parentName(), space);
        }
    }
}

bool VolumeManager::registerAction(Actions::Action* action)
{
    /* A duplicate isn't accepted */
    if (actionstack.contains(action)) {
        return false;
    }
    
    switch (action->actionType()) {
        case Action::FormatPartition: {
            Actions::FormatPartitionAction* fpa = dynamic_cast< Actions::FormatPartitionAction* >(action);
            DeviceModified* p = searchDeviceByName(fpa->partition());
            
            if (!p || !p->deviceType() == DeviceModified::PartitionDevice) {
                qDebug() << "errore, nome non valido";
                return false;
            }
            
            Partition* volume = dynamic_cast<Partition *>(p);
            volume->setFilesystem(fpa->filesystem()); /* FIXME: check if the filesystem is supported */
            break;
        }
        
        case Action::CreatePartition: {
            Actions::CreatePartitionAction* cpa = dynamic_cast< Actions::CreatePartitionAction* >(action);
            if (!volumeTrees.contains(cpa->disk())) {
                qDebug() << "unexistent disk";
                return false;
            }
            
            VolumeTree tree = volumeTrees[cpa->disk()];
            
            if (!tree.d->splitCreationContainer(cpa->offset(), cpa->size())) {
                qDebug() << "could not split";
                return false;
            }
            
            Partition* newPartition = new Partition(cpa);
            tree.addDevice(cpa->disk(), newPartition);
        }
        
        case Action::RemovePartition: {
            Actions::RemovePartitionAction* rpa = dynamic_cast< Actions::RemovePartitionAction* >(action);
            
            /* FIXME: put this check in the next call without doing the same thing twice */
            if (!searchDeviceByName(rpa->partition())) {
                qDebug() << "partition not found";
                return false;
            }
            VolumeTree tree = searchTreeWithDevice(rpa->partition());
            tree.d->mergeAndDelete(rpa->partition());
            break;
        }
        
        case Action::ResizePartition: {
            Actions::ResizePartitionAction* rpa = dynamic_cast< Actions::ResizePartitionAction* >(action);
            
            VolumeTree tree = searchTreeWithDevice(rpa->partition());
            VolumeTreeItem* itemToResize = tree.searchNode(rpa->partition());
            
            if (!itemToResize) {
                qDebug() << "partition doesn't exist";
            }
            
            Partition* toResize = dynamic_cast< Partition* >(itemToResize->volume());
            DeviceModified* leftDevice = tree.d->leftDevice(itemToResize);
            DeviceModified* rightDevice = tree.d->rightDevice(itemToResize);
            
            qlonglong boundary = 0;
            
            if (rpa->newSize() == 0) {
                qDebug() << "resizing to zero isn't allowed";
                return false;
            }
            
            if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
                boundary = toResize->rightBoundary();
            } else {
                boundary = rightDevice->offset() + rightDevice->size();
            }
            
            if ((toResize->offset() + rpa->newSize()) >= boundary) {
                qlonglong difference = toResize->offset() + rpa->newSize() - boundary;
                
                if ((toResize->offset() - rpa->newOffset()) < difference) {
                    qDebug() << "out of bounds";
                    return false;
                }
                
                movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
                resizePartition(toResize, rpa->newSize(), rightDevice, tree);
            }
            else if (rpa->newOffset() + toResize->size() >= boundary) {
                qlonglong difference = rpa->newOffset() + toResize->size() - boundary; /* of how much? */
                
                /*
                 * If the size is reduced of at least that quantity
                 * then the partition will stay in its limit, therefore the operation is allowed.
                 * Otherwise, the exception below is thrown.
                 */
                if ((toResize->size() - rpa->newSize()) < difference) {
                    qDebug() << "out of bounds";
                    return false;
                }
                
                /*
                 * To avoid problems with further calculations, first change the size and then the offset,
                 * so no limit is surpassed even temporarily.
                 */
                resizePartition(toResize, rpa->newSize(), rightDevice, tree);
                movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
            }
            else {
                resizePartition(toResize, rpa->newSize(), rightDevice, tree);
                movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
            }
            
            break;
        }
        
        default:
            break;
    }
    
    actionstack.push(action);
    return true;
}

void VolumeManager::undo()
{
}

void VolumeManager::redo()
{
}

void VolumeManager::resizePartition(Partition* partition,
                                    qlonglong ns,
                                    DeviceModified* rightDevice,
                                    VolumeTree& tree)
{  
    if (ns == -1) {
        return;
    }
    qulonglong newSize = (qulonglong)ns;
    
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = partition->offset() + newSize;
        qulonglong spaceSize = partition->size() - newSize;
        FreeSpace* freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
        qDebug() << "RESIZE: creato a destra spazio di dimensione" << spaceSize;
        tree.addDevice(partition->parentName(), freeSpaceRight);
    } else {
        FreeSpace* spaceRight = dynamic_cast< FreeSpace* >(rightDevice);
        qulonglong rightOffset = partition->offset() + newSize;
        qulonglong rightSize = spaceRight->size() - (newSize - partition->size());
        
        qDebug() << "RESIZE: a destra lo spazio passa da offset=" << spaceRight->offset() << "e size=" << spaceRight->size() << "a" << rightOffset << "e" << rightSize;
        
        if (rightSize == 0) {
            tree.removeDevice(spaceRight->name());
        } else {
            spaceRight->setOffset(rightOffset);
            spaceRight->setSize(rightSize);
        }
    }
    
    qDebug() << "partition resized from" << partition->size() << "to" << newSize;
    partition->setSize(newSize);
}

void VolumeManager::movePartition(Partition* partition,
                                  qlonglong no,
                                  DeviceModified* leftDevice,
                                  DeviceModified* rightDevice,
                                  DeviceModified* parent,
                                  VolumeTree& tree)
{
    if (no == -1) {
        return;
    }
    
    qulonglong newOffset = (qulonglong)no;
    qulonglong oldOffset = partition->offset();
    FreeSpace *freeSpaceRight = 0;
    FreeSpace *freeSpaceLeft = 0;
    
    if (!leftDevice || leftDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        
        /* This is an error for sure */
        if (newOffset < oldOffset) {
            qDebug() << "out of bounds";
            return;
        }
        
        /*
         * Creates a new Device object for the space left on the left (no pun intended)
         * If this will be the first Device in the disk pays attention to leave the first MB reserved for MBR
         * (later we should see how this works for GPT too)
         */
        qulonglong spaceOffset = (leftDevice) ? (leftDevice->rightBoundary()) : parent->offset();
        qulonglong spaceSize = newOffset - oldOffset;
        freeSpaceLeft = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
    } else { /* there's some free space immediately before: changes its size accordingly */
        
        /* This one too */
        if (newOffset < leftDevice->offset()) {
            qDebug() << "out of bounds";
        }
        
        qulonglong leftSize = leftDevice->size() - (oldOffset - newOffset);
        
        /*
         * In this case we completely filled up the space available
         */
        if (leftSize == 0) {
            tree.removeDevice(leftDevice->name());
        } else {
            leftDevice->setSize(leftSize);
        }
    }
    
    /*
     * Moving a partition around affects of course the device on the right too
     * NOTE for future developers: we can't use resizePartition for this because in this case
     * the size isn't changing, just the position of the partition.
     */
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = newOffset + partition->size();
        qulonglong spaceSize = oldOffset - newOffset;
        
        qDebug() << "MOVE: created on the right a new device with offset=" << spaceOffset << "and size=" << spaceSize;
        
        freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
        tree.addDevice(partition->parentName(), freeSpaceRight);
    } else {
        qulonglong rightOffset = newOffset + partition->size();
        qulonglong rightSize = rightDevice->size() - (newOffset - oldOffset);
        qDebug() << "MOVE: the unallocated space on the left changed size from" << rightDevice->size() << "to" << rightSize;
        qDebug() << "MOVE: the unallocated space on the left changed from offset" << rightDevice->offset() << "to" << rightOffset;
        
        if (rightSize == 0) {
            tree.removeDevice(rightDevice->name());
        } else {
            rightDevice->setOffset(rightOffset);
            rightDevice->setSize(rightSize);
        }
    }
    
    partition->setOffset(newOffset);
    
    /*
     * NOTE: the device must be added here and not right after it has been created because
     * its position in the list is determined based on its offset (to have a sorted list of devices).
     * But the offset of "partition" has changed above so we may have errors in the sorting.
     */
    if (freeSpaceLeft) {
        tree.addDevice(partition->parentName(), freeSpaceLeft);
    }
}

QList< VolumeTree > VolumeManager::allDiskTrees() const
{
    return volumeTrees.values();
}

VolumeTree VolumeManager::diskTree(const QString& diskName) const
{
    return volumeTrees[diskName];
}

bool VolumeManager::apply()
{
    executer = new ActionExecuter(actionstack.list());
    qDebug() << executer->valid();
    
    ActionExecuter* e2 = new ActionExecuter(actionstack.list());
    qDebug() << e2->valid();
    
    delete executer;
    delete e2;
    
    return true;
}

DeviceModified* VolumeManager::searchDeviceByName(const QString& name)
{
    DeviceModified* dev = 0;
    
    foreach (const VolumeTree& tree, volumeTrees.values()) {
        dev = tree.searchDevice(name);
        
        if (dev) {
            break;
        }
    }
    
    return dev;
}

VolumeTree VolumeManager::searchTreeWithDevice(const QString& name)
{
    
    foreach (VolumeTree tree, volumeTrees.values()) {
        if (tree.searchDevice(name)) {
            return tree;
        }
    }
    
    /* FIXME: exception/error */
    return VolumeTree();
}

}
}