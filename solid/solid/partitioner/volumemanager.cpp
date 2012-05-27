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
#include <solid/partitioner/actions/createpartitionaction.h>
#include <solid/partitioner/actions/removepartitionaction.h>
#include <solid/partitioner/actions/resizepartitionaction.h>
#include <solid/partitioner/actions/createpartitiontableaction.h>
#include <solid/partitioner/actions/modifypartitionaction.h>
#include <solid/partitioner/actions/removepartitiontableaction.h>
#include <solid/partitioner/utils/partitioningerror.h>
#include <solid/device.h>
#include <kglobal.h>

namespace Solid
{   
namespace Partitioner
{

using namespace Devices;
using namespace Utils;

class VolumeManager::Private
{
public:
    Private()
        : executer(0)
    {}
    
    ~Private()
    {}
            
    /* Detection of drives, partitions, and free space */
    void detectDevices();
    
    /* Searches a device by its name in all trees. Returns NULL if not found. */
    DeviceModified* searchDeviceByName(const QString &);
    VolumeTree searchTreeWithDevice(const QString &);
    
    void resizePartition(Partition *, qlonglong, DeviceModified *, VolumeTree &);
    void movePartition(Partition *, qlonglong, DeviceModified *, DeviceModified *, DeviceModified *, VolumeTree &);
    bool setPartitionTableScheme(const QString &, Utils::PTableType);
    
    QMap<QString, VolumeTree> volumeTrees;
    ActionStack actionstack;
    ActionExecuter* executer;
    
    PartitioningError error;
};

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
    : d( new Private )
{
    Q_ASSERT(!s_volumemanager->q);
    s_volumemanager->q = this;
    
    d->detectDevices();
}

VolumeManager::~VolumeManager()
{
    delete d;
}

VolumeManager* VolumeManager::instance()
{
    if (!s_volumemanager->q) {
        new VolumeManager;
    }
    
    return s_volumemanager->q;
}

bool VolumeManager::registerAction(Actions::Action* action)
{
    /* A duplicate isn't accepted */
    if (d->actionstack.contains(action)) {
        d->error.setType(PartitioningError::DuplicateActionError);
        d->error.arg(action->description());
        return false;
    }
    
    switch (action->actionType()) {
        case Action::FormatPartition: {
            Actions::FormatPartitionAction* fpa = dynamic_cast< Actions::FormatPartitionAction* >(action);
            DeviceModified* p = d->searchDeviceByName(fpa->partition());
            Utils::Filesystem fs = fpa->filesystem();

            if (!fs.unsupportedFlags().isEmpty()) {
                d->error.setType(PartitioningError::FilesystemFlagsError);
                d->error.arg(fpa->filesystem().unsupportedFlags().join(", "));
                return false;
            }
            
            if (!p) {
                d->error.setType(PartitioningError::PartitionNotFoundError);
                d->error.arg(fpa->partition());
                return false;
            }
            
            if (p->deviceType() != DeviceModified::PartitionDevice) {
                d->error.setType(PartitioningError::WrongDeviceTypeError);
                d->error.arg("partition");
                return false;
            }
            
            Partition* volume = dynamic_cast<Partition *>(p);
            volume->setFilesystem(fs);
            break;
        }
        
        case Action::CreatePartition: {
            Actions::CreatePartitionAction* cpa = dynamic_cast< Actions::CreatePartitionAction* >(action);
            
            if (!d->volumeTrees.contains(cpa->disk())) {
                d->error.setType(PartitioningError::DiskNotFoundError);
                d->error.arg(cpa->disk());
                return false;
            }
            
            VolumeTree tree = d->volumeTrees[cpa->disk()];
            Disk* disk = dynamic_cast< Disk* >( tree.searchDevice(cpa->disk()) );
            Utils::PTableType scheme = disk->partitionTableScheme();

            if (scheme == Utils::None) {
                d->error.setType(PartitioningError::NoPartitionTableError);
                d->error.arg( cpa->disk() );
                return false;
            }
            
            if (tree.partitions().count() == 4 && cpa->partitionType() != Logical) {
                d->error.setType(PartitioningError::ExceedingPrimariesError);
                d->error.arg( cpa->disk() );
                return false;
            }
            
            if (!tree.d->splitCreationContainer(cpa->offset(), cpa->size())) {
                d->error.setType(PartitioningError::ContainerNotFoundError);
                d->error.arg(QString::number(cpa->offset()));
                d->error.arg(QString::number(cpa->size()));
                return false;
            }

            foreach (const QString& flagSet, cpa->flags()) {
                if (!Utils::PartitionTableUtils::instance()->supportedFlags(scheme).contains(flagSet)) {
                    d->error.setType(PartitioningError::PartitionFlagsError);
                    d->error.arg(flagSet);
                    return false;
                }
            }

            Partition* newPartition = new Partition(cpa);
            tree.d->addDevice(cpa->disk(), newPartition);
        }
        
        case Action::RemovePartition: {
            Actions::RemovePartitionAction* rpa = dynamic_cast< Actions::RemovePartitionAction* >(action);
            
            VolumeTree tree = d->searchTreeWithDevice(rpa->partition());
            if (!tree.d) {
                d->error.setType(PartitioningError::PartitionNotFoundError);
                d->error.arg(rpa->partition());
                return false;
            }
            
            DeviceModified* dev = d->searchDeviceByName(rpa->partition());
            if (dev->deviceType() != DeviceModified::PartitionDevice) {
                d->error.setType(PartitioningError::WrongDeviceTypeError);
                d->error.arg("partition");
            }
            
            tree.d->mergeAndDelete(rpa->partition());
            break;
        }
        
        case Action::ResizePartition: {
            Actions::ResizePartitionAction* rpa = dynamic_cast< Actions::ResizePartitionAction* >(action);
            
            VolumeTree tree = d->searchTreeWithDevice(rpa->partition());
            
            if (!tree.d) {
                d->error.setType(PartitioningError::PartitionNotFoundError);
                d->error.arg(rpa->partition());
                return false;
            }
            
            VolumeTreeItem* itemToResize = tree.searchNode(rpa->partition());            
            Partition* toResize = dynamic_cast< Partition* >(itemToResize->volume());
            DeviceModified* leftDevice = tree.d->leftDevice(itemToResize);
            DeviceModified* rightDevice = tree.d->rightDevice(itemToResize);
            
            qlonglong boundary = 0;
            
            if (rpa->newSize() == 0) {
                d->error.setType(PartitioningError::ResizingToZeroError);
                return false;
            }
            
            if (rpa->newSize() == -1 && rpa->newOffset() == -1) {
                d->error.setType(PartitioningError::ResizingToTheSameError);
                d->error.arg(rpa->partition());
                return false;
            }
            
            if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
                boundary = toResize->rightBoundary();
            } else {
                boundary = rightDevice->offset() + rightDevice->size();
            }
            
            if ((toResize->offset() + rpa->newSize()) > boundary) {
                qlonglong difference = toResize->offset() + rpa->newSize() - boundary;
                
                if ((toResize->offset() - rpa->newOffset()) < difference) {
                    d->error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                d->movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
                d->resizePartition(toResize, rpa->newSize(), rightDevice, tree);
            }
            else if (rpa->newOffset() + toResize->size() >= boundary) {
                qlonglong difference = rpa->newOffset() + toResize->size() - boundary; /* of how much? */
                
                /*
                 * If the size is reduced of at least that quantity
                 * then the partition will stay in its limit, therefore the operation is allowed.
                 * Otherwise, the exception below is thrown.
                 */
                if ((toResize->size() - rpa->newSize()) < difference) {
                    d->error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                /*
                 * To avoid problems with further calculations, first change the size and then the offset,
                 * so no limit is surpassed even temporarily.
                 */
                d->resizePartition(toResize, rpa->newSize(), rightDevice, tree);
                d->movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
            }
            else {
                d->resizePartition(toResize, rpa->newSize(), rightDevice, tree);
                d->movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
            }
            
            break;
        }
        
        case Action::ModifyPartition: {
            Actions::ModifyPartitionAction* mpa = dynamic_cast< Actions::ModifyPartitionAction* >(action);
            DeviceModified* device = d->searchDeviceByName( mpa->partition() );
            VolumeTree tree = d->searchTreeWithDevice( mpa->partition() );
            
            if (!device) {
                d->error.setType(PartitioningError::PartitionNotFoundError);
                d->error.arg(mpa->partition());
                return false;
            }
            
            if (device->deviceType() != DeviceModified::PartitionDevice) {
                d->error.setType(PartitioningError::WrongDeviceTypeError);
                d->error.arg("partition");
                return false;
            }
            
            Partition* p = dynamic_cast< Partition* >(device);
            
            if (mpa->isLabelChanged()) {
                p->setLabel( mpa->label() );
            }

            Disk* parent = dynamic_cast< Disk* >( tree.searchNode( mpa->partition() )->parent()->volume() );
            QStringList supportedFlags = PartitionTableUtils::instance()->supportedFlags( parent->partitionTableScheme() );

            foreach (const QString& flag, mpa->flagsToSet()) {
                if (!supportedFlags.contains(flag)) {
                    d->error.setType(PartitioningError::PartitionFlagsError);
                    d->error.arg(flag);
                    return false;
                }

                p->setFlag(flag);
            }

            foreach (const QString& flag, mpa->flagsToUnset()) {
                if (!supportedFlags.contains(flag)) {
                    d->error.setType(PartitioningError::PartitionFlagsError);
                    d->error.arg(flag);
                    return false;
                }

                p->unsetFlag(flag);
            }
            break;
        }
        
        case Action::CreatePartitionTable: {
            Actions::CreatePartitionTableAction* cpta = dynamic_cast< Actions::CreatePartitionTableAction* >(action);
            
            if (!d->setPartitionTableScheme(cpta->disk(), cpta->partitionTableScheme())) {
                return false;
            }
            
            break;
        }
        
        case Action::RemovePartitionTable: {
            Actions::RemovePartitionTableAction* rpta = dynamic_cast< Actions::RemovePartitionTableAction* >(action);
            
            if (!d->setPartitionTableScheme(rpta->disk(), Utils::None)) {
                return false;
            }
            
            break;
        }
        
        default:
            break;
    }
    
    d->actionstack.push(action);
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
    return d->volumeTrees.values();
}

VolumeTree VolumeManager::diskTree(const QString& diskName) const
{
    return d->volumeTrees[diskName];
}

bool VolumeManager::apply()
{
    d->executer = new ActionExecuter(d->actionstack.list());
    qDebug() << d->executer->valid();
    
    ActionExecuter* e2 = new ActionExecuter(d->actionstack.list());
    qDebug() << e2->valid();
    
    delete d->executer;
    delete e2;
    
    return true;
}

QString VolumeManager::errorDescription() const
{
    return d->error.description();
}

void VolumeManager::Private::detectDevices()
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
                
                tree.d->addDevice(volume->parentName(), volume);
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
            
            tree.d->addDevice(parentName, volume);
        }
    }
    
    /*
     * Detection of blocks of free space.
     */
    foreach (VolumeTree disk, volumeTrees.values()) {
        foreach (Devices::FreeSpace* space, Device::freeSpaceOfDisk(disk)) {
            disk.d->addDevice(space->parentName(), space);
        }
        
        disk.print();
    }
}

void VolumeManager::Private::resizePartition(Partition* partition,
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
        tree.d->addDevice(partition->parentName(), freeSpaceRight);
    } else {
        FreeSpace* spaceRight = dynamic_cast< FreeSpace* >(rightDevice);
        qulonglong rightOffset = partition->offset() + newSize;
        qulonglong rightSize = spaceRight->size() - (newSize - partition->size());
        
        qDebug() << "RESIZE: a destra lo spazio passa da offset=" << spaceRight->offset() << "e size=" << spaceRight->size() << "a" << rightOffset << "e" << rightSize;
        
        if (rightSize == 0) {
            tree.d->removeDevice(spaceRight->name());
        } else {
            spaceRight->setOffset(rightOffset);
            spaceRight->setSize(rightSize);
        }
    }
    
    qDebug() << "partition resized from" << partition->size() << "to" << newSize;
    partition->setSize(newSize);
}

void VolumeManager::Private::movePartition(Partition* partition,
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
            error.setType(PartitioningError::ResizeOutOfBoundsError);
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
            error.setType(PartitioningError::ResizeOutOfBoundsError);
        }
        
        qulonglong leftSize = leftDevice->size() - (oldOffset - newOffset);
        
        /*
         * In this case we completely filled up the space available
         */
        if (leftSize == 0) {
            tree.d->removeDevice(leftDevice->name());
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
        tree.d->addDevice(partition->parentName(), freeSpaceRight);
    } else {
        qulonglong rightOffset = newOffset + partition->size();
        qulonglong rightSize = rightDevice->size() - (newOffset - oldOffset);
        qDebug() << "MOVE: the unallocated space on the left changed size from" << rightDevice->size() << "to" << rightSize;
        qDebug() << "MOVE: the unallocated space on the left changed from offset" << rightDevice->offset() << "to" << rightOffset;
        
        if (rightSize == 0) {
            tree.d->removeDevice(rightDevice->name());
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
        tree.d->addDevice(partition->parentName(), freeSpaceLeft);
    }
}

bool VolumeManager::Private::setPartitionTableScheme(const QString& diskName, PTableType scheme)
{
    VolumeTree tree = volumeTrees[diskName];

    if (!tree.d) {
        error.setType(PartitioningError::DiskNotFoundError);
        error.arg(diskName);
        return false;
    }
    
    /*
     * After removing or changing the partition table scheme, remove all the partitions.
     * For now transitioning from one scheme to another isn't supported as it's potentially dangerous,
     * so applications should warn the user about losing everything before authorizing this operation.
     */ 
    VolumeTreeItem* diskNode = tree.searchNode(diskName);
    Disk* disk = dynamic_cast< Disk* >( diskNode->volume() );
    disk->setPartitionTableScheme(scheme);
    diskNode->clearChildren();
    
    /*
     * Instead, add a big block of free space as the only disk's child in the tree.
     */
    FreeSpace* bigFreeSpace = new FreeSpace(disk->offset(), disk->size(), disk->name());
    diskNode->addChild(bigFreeSpace);
    return true;
}


DeviceModified* VolumeManager::Private::searchDeviceByName(const QString& name)
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

VolumeTree VolumeManager::Private::searchTreeWithDevice(const QString& name)
{
    
    foreach (VolumeTree tree, volumeTrees.values()) {
        if (tree.searchDevice(name)) {
            return tree;
        }
    }
    
    return VolumeTree();
}

}
}
