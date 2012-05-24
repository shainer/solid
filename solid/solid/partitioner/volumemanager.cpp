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
#include <solid/device.h>
#include <kglobal.h>

namespace Solid
{   
namespace Partitioner
{
    
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
            Actions::FormatPartitionAction* fpa = dynamic_cast<Actions::FormatPartitionAction *>(action);
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
            Actions::CreatePartitionAction* cpa = dynamic_cast<Actions::CreatePartitionAction *>(action);
            if (!volumeTrees.contains(cpa->disk())) {
                qDebug() << "unexistent disk";
                return false;
            }
            
            VolumeTree tree = volumeTrees[cpa->disk()];
            
            tree.print();
            
            if (!tree.splitCreationContainer(cpa->offset(), cpa->size())) {
                qDebug() << "could not split";
                return false;
            }
            
            Partition* newPartition = new Partition(cpa);
            tree.addDevice(cpa->disk(), newPartition);
            tree.print();
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

}
}