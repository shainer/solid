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
#include <solid/partitioner/utils/utils.h>
#include <solid/device.h>
#include <backends/udisks/udisksmanager.h>
#include <kglobal.h>
#include <solid/predicate.h>

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
        , deviceManager( new Backends::UDisks::UDisksManager(0) )
    {}
    
    ~Private()
    {}
            
    /* Detection of drives, partitions, and free space */
    void detectDevices();
    
    /* Adds a new disk to the system */
    Disk* addDisk(Solid::StorageDrive *, const QString &);
    
    /* Detection of partitions and free space blocks in a disk. All previous detections are deleted. */
    void detectPartitionsOfDisk(const QString &);
    void detectFreeSpaceOfDisk(const QString &);
    
    /* Searches a device by its name in all trees. Returns NULL if not found. */
    DeviceModified* searchDeviceByName(const QString &);
    
    /* Returns the tree which contains the given device, or an invalid object if not found. */
    VolumeTree searchTreeWithDevice(const QString &);
    
    /* Resize a partition given its parent. */
    void resizePartition(Partition *, qlonglong, DeviceModified *, VolumeTree &);
    
    /* Changes the initial offset of a partition. */
    void movePartition(Partition *, qlonglong, DeviceModified *, DeviceModified *, DeviceModified *, VolumeTree &);
    
    /* Perform necessary checks when creating a new partition with either MBR or GPT. */
    bool mbrPartitionChecks(Actions::CreatePartitionAction *);
    bool gptPartitionChecks(Actions::CreatePartitionAction *);
    
    /* Set a new ptable scheme for a disk, deleting all the partitions. */
    bool setPartitionTableScheme(const QString &, Utils::PTableType);
    
    QMap<QString, VolumeTree> volumeTrees;
    ActionStack actionstack;
    ActionExecuter* executer;
    Ifaces::DeviceManager* deviceManager; /* FIXME: to be destroyed */
    
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
    
    QObject::connect(d->deviceManager, SIGNAL(deviceAdded(QString)), this, SLOT(doDeviceAdded(QString)));
    QObject::connect(d->deviceManager, SIGNAL(deviceRemoved(QString)), this, SLOT(doDeviceRemoved(QString)));
    d->detectDevices();
}

VolumeManager::~VolumeManager()
{
    d->volumeTrees.clear();
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
            VolumeTree tree = d->searchTreeWithDevice(fpa->partition());
            DeviceModified* p = tree.searchDevice(fpa->partition());
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
            
            if (volume->usage() == StorageVolume::Other) {
                d->error.setType(PartitioningError::CannotFormatPartition);
                d->error.arg(volume->name());
                return false;
            }
            
            volume->setFilesystem(fs);
            
            fpa->setOwnerDisk(tree.root());
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

            switch (scheme) {
                case Utils::None: {
                    d->error.setType(PartitioningError::NoPartitionTableError);
                    d->error.arg( cpa->disk() );
                    return false;
                }
                
                case Utils::MBR: {
                    if (!d->mbrPartitionChecks(cpa)) {
                        return false;
                    }
                    
                    break;
                }
                
                case Utils::GPT: {
                    if (!d->gptPartitionChecks(cpa)) {
                        return false;
                    }
                    
                    break;
                }
                
                default:
                    break;
            }

            foreach (const QString& flagSet, cpa->flags()) {
                if (!Utils::PartitionTableUtils::instance()->supportedFlags(scheme).contains(flagSet)) {
                    d->error.setType(PartitioningError::PartitionFlagsError);
                    d->error.arg(flagSet);
                    return false;
                }
            }
            
            if (!tree.d->splitCreationContainer(cpa->offset(), cpa->size())) {
                d->error.setType(PartitioningError::ContainerNotFoundError);
                d->error.arg(QString::number(cpa->offset()));
                d->error.arg(QString::number(cpa->size()));
                return false;
            }

            Partition* newPartition = new Partition(cpa);
            tree.d->addDevice(cpa->disk(), newPartition);
            
            cpa->setOwnerDisk(disk);
            break;
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
            rpa->setOwnerDisk(tree.root());
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
            
            if (leftDevice && leftDevice->deviceType() != DeviceModified::FreeSpaceDevice && rpa->newOffset() < toResize->offset()) {
                d->error.setType(PartitioningError::ResizeOutOfBoundsError);
                return false;
            }
            
    
            /* Special checks for extendend partitions */
            DeviceModified* firstLogical = 0;
            DeviceModified* lastLogical = 0;
            if (toResize->partitionType() == Extended) {
                lastLogical = tree.extendedNode()->children().last()->volume();
                firstLogical = tree.extendedNode()->children().first()->volume();
                
                if (rpa->newSize() < toResize->size() &&
                   (lastLogical->deviceType() != DeviceModified::FreeSpaceDevice ||
                    toResize->offset() + rpa->newSize() < lastLogical->offset())) {
                    d->error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                if (rpa->newOffset() > toResize->offset() &&
                    (firstLogical->deviceType() != DeviceModified::FreeSpaceDevice ||
                    toResize->offset() > firstLogical->rightBoundary())) {
                    d->error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
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
            
            rpa->setOwnerDisk(tree.root());
            tree.print();
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
            
            mpa->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::CreatePartitionTable: {
            Actions::CreatePartitionTableAction* cpta = dynamic_cast< Actions::CreatePartitionTableAction* >(action);
            
            if (!d->setPartitionTableScheme(cpta->disk(), cpta->partitionTableScheme())) {
                return false;
            }
            
            VolumeTree tree = d->volumeTrees[cpta->disk()];
            cpta->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::RemovePartitionTable: {
            Actions::RemovePartitionTableAction* rpta = dynamic_cast< Actions::RemovePartitionTableAction* >(action);
            
            if (!d->setPartitionTableScheme(rpta->disk(), Utils::None)) {
                return false;
            }
            
            VolumeTree tree = d->volumeTrees[rpta->disk()];
            rpta->setOwnerDisk(tree.root());
            break;
        }
        
        default:
            break;
    }
    
    d->actionstack.push(action);
    return true;
}

void VolumeManager::undo()
{}

void VolumeManager::redo()
{}

/*
 * FIXME: in order for this to work, we must be sure that when this slot is called, it isn't
 * called again until the previous instance hasn't finished execution.
 */
void VolumeManager::doDeviceAdded(QString udi)
{
    Predicate pred1( DeviceInterface::StorageDrive, "udi", udi );
    Predicate pred2( DeviceInterface::StorageVolume, "udi", udi );
    QList<Device> drives = Device::listFromQuery(pred1);
        
    if (!drives.isEmpty()) {
        StorageDrive* drive = drives.first().as<StorageDrive>();
        Disk* disk = d->addDisk(drive, udi);
        
        d->detectFreeSpaceOfDisk(disk->name());
        d->actionstack.removeActionsOfDisk(disk->name());
        
        VolumeTree diskTree = d->volumeTrees[disk->name()];
        emit deviceAdded(diskTree);
    }
    else {
        QList<Device> volumes = Device::listFromQuery(pred2);
        
        if (volumes.isEmpty()) {
            return;
        }
        
        QString diskName = volumes.first().parentUdi();
        VolumeTree diskTree = d->volumeTrees[diskName];
        
        if (!diskTree.d) {
            return;
        }

        d->detectPartitionsOfDisk(volumes.first().parentUdi());
        d->detectFreeSpaceOfDisk(diskName);
        
        d->actionstack.removeActionsOfDisk(diskName);
        emit deviceAdded(diskTree);
    }
}

void VolumeManager::doDeviceRemoved(QString udi)
{
    Predicate pred1( DeviceInterface::StorageDrive, "udi", udi );
    Predicate pred2( DeviceInterface::StorageVolume, "udi", udi );
    QList<Device> drives = Device::listFromQuery(pred1);
    QString diskName;
    
    if (!drives.isEmpty()) {
        Device drive = drives.first();
        diskName = drive.udi();
    }
    else {
        QList<Device> volumes = Device::listFromQuery(pred2);
        
        if (volumes.isEmpty()) {
            return;
        }
        
        QString diskName = volumes.first().parentUdi();
    }
    
    d->volumeTrees.remove(diskName);
    d->actionstack.removeActionsOfDisk(diskName);
    emit deviceRemoved(diskName);
}

QMap<QString, VolumeTree> VolumeManager::allDiskTrees() const
{
    return d->volumeTrees;
}

VolumeTree VolumeManager::diskTree(const QString& diskName) const
{
    return d->volumeTrees[diskName];
}

bool VolumeManager::apply()
{
    return true;
}

QString VolumeManager::errorDescription() const
{
    return d->error.description();
}

QList< Action* > VolumeManager::registeredActions() const
{
    return d->actionstack.list();
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
            addDisk(drive, udi);
            
            /* FIXME */
            if (!udi.contains("loop")) {
                detectPartitionsOfDisk(udi);
                detectFreeSpaceOfDisk(udi);
            }
        }
    }
    
    foreach (VolumeTree disk, volumeTrees.values()) {
        disk.print();
    }
}

Disk* VolumeManager::Private::addDisk(StorageDrive* drive, const QString& udi)
{
    Devices::Disk* disk = new Devices::Disk( drive );
    disk->setName(udi);
    
    VolumeTree tree( disk );
    volumeTrees.insert(disk->name(), tree);
    return disk;
}

void VolumeManager::Private::detectPartitionsOfDisk(const QString& parentUdi)
{    
    QList<Device> devices = Device::listFromType(DeviceInterface::StorageVolume, parentUdi);
    Partition* extended = 0;
    VolumeTree tree = volumeTrees[parentUdi];
    tree.d->removeAllOfType(DeviceModified::PartitionDevice);
    
    /*
     * For now doesn't consider all the volumes not supported.
     */
    for (QList<Device>::iterator it = devices.begin(); it != devices.end(); it++) {
        StorageVolume* volume = it->as<StorageVolume>();
        
        if (volume->usage() > StorageVolume::FileSystem) {
            devices.erase(it);
        }
    }
    
    foreach (Device device, devices) {
        StorageVolume* volume = device.as<StorageVolume>();

        if (volume->uuid().isEmpty()) {
            extended = new Partition(volume);
            extended->setPartitionType(Extended);
            extended->setName(device.udi());
            extended->setParentName(parentUdi);
            
            tree.d->addDevice(parentUdi, extended);
        }
    }
    
    foreach (Device device, devices) {
        StorageVolume* volume = device.as<StorageVolume>();
        QString parentName = parentUdi;
        
        if (volume->uuid().isEmpty()) {
            continue;
        }
        
        Partition* part = new Partition(volume);
        
        if (extended && (part->offset() >= extended->offset() && part->rightBoundary() <= extended->rightBoundary())) {
            part->setPartitionType(Logical);
            parentName = extended->name();
        } else {
            part->setPartitionType(Primary);
        }
        
        part->setName(device.udi());
        part->setParentName(parentName);
        tree.d->addDevice(parentName, part);
    }    
}

void VolumeManager::Private::detectFreeSpaceOfDisk(const QString& diskName)
{
    VolumeTree tree = volumeTrees[diskName];
    tree.d->removeAllOfType(DeviceModified::FreeSpaceDevice);
    
    foreach (FreeSpace* space, Device::freeSpaceOfDisk(tree)) {
        tree.d->addDevice(diskName, space);
    }
}

void VolumeManager::Private::resizePartition(Partition* partition,
                                    qlonglong ns,
                                    DeviceModified* rightDevice,
                                    VolumeTree& tree)
{  
    if (ns == -1 || ns == partition->size()) {
        return;
    }
    qulonglong newSize = (qulonglong)ns;
    qulonglong spaceBetween = (partition->partitionType() == Logical ? SPACE_BETWEEN_LOGICALS : 0);
    
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = partition->offset() + newSize + spaceBetween;
        qulonglong spaceSize = partition->size() - newSize - spaceBetween;
        
        if (spaceSize > 0) {
            FreeSpace* freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
            tree.d->addDevice(partition->parentName(), freeSpaceRight);
        }
    } else {
        FreeSpace* spaceRight = dynamic_cast< FreeSpace* >(rightDevice);
        qulonglong rightOffset = partition->offset() + newSize;
        qulonglong rightSize = spaceRight->size() - (newSize - partition->size());
                
        if (rightSize == 0) {
            tree.d->removeDevice(spaceRight->name());
        } else {
            spaceRight->setOffset(rightOffset);
            spaceRight->setSize(rightSize);
        }
    }
    
    if (partition->partitionType() == Extended) {
        DeviceModified* lastLogical = tree.extendedNode()->children().last()->volume();
        
        if (lastLogical->deviceType() == DeviceModified::FreeSpaceDevice) {
            qulonglong logSize = lastLogical->size() + (newSize - partition->size());
            
            if (logSize == 0) {
                tree.d->removeDevice(lastLogical->name());
            } else {
                lastLogical->setSize(logSize);
            }
        } else {
            qulonglong lastOffset = lastLogical->rightBoundary();
            qulonglong lastSize = partition->offset() + newSize - lastOffset;
            
            FreeSpace* last = new FreeSpace(lastOffset, lastSize, partition->name());
            tree.d->addDevice(partition->name(), last);
        }
    }
    
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
    
    qulonglong spaceBetween = (partition->partitionType() == Logical ? SPACE_BETWEEN_LOGICALS : 0);
    qulonglong newOffset = (qulonglong)no;
    qulonglong oldOffset = partition->offset();
    FreeSpace *freeSpaceRight = 0;
    FreeSpace *freeSpaceLeft = 0;
    
    if (!leftDevice || leftDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        /*
         * Creates a new Device object for the space left on the left (no pun intended)
         */
        qulonglong spaceOffset = (leftDevice) ? (leftDevice->rightBoundary()) : parent->offset() + spaceBetween;
        qulonglong spaceSize = newOffset - oldOffset - spaceBetween;
        
        if (spaceSize > 0) {
            freeSpaceLeft = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
        }
    } else { /* there's some free space immediately before: changes its size accordingly */
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
        qulonglong spaceOffset = newOffset + partition->size() + spaceBetween;
        qulonglong spaceSize = oldOffset - newOffset - spaceBetween;
                
        if (spaceSize > 0) {
            freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
            tree.d->addDevice(partition->parentName(), freeSpaceRight);
        }
    } else {
        qulonglong rightOffset = newOffset + partition->size();
        qulonglong rightSize = rightDevice->size() - (newOffset - oldOffset);
        
        if (rightSize == 0) {
            tree.d->removeDevice(rightDevice->name());
        } else {
            rightDevice->setOffset(rightOffset);
            rightDevice->setSize(rightSize);
        }
    }
    
    if (partition->partitionType() == Extended) {
        DeviceModified* firstLogical = tree.extendedNode()->children().first()->volume();
        
        if (firstLogical->deviceType() == DeviceModified::FreeSpaceDevice) {
            qulonglong logSize = firstLogical->size() - (newOffset - partition->offset());
            
            if (logSize == 0) {
                tree.d->removeDevice(firstLogical->name());
            } else {
                firstLogical->setSize(logSize);
            }
        } else {
            qulonglong firstOffset = partition->offset();
            qulonglong firstSize = partition->offset() - newOffset;
            
            FreeSpace* first = new FreeSpace(firstOffset, firstSize, partition->name());
            tree.d->addDevice(partition->name(), first);
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

bool VolumeManager::Private::mbrPartitionChecks(Actions::CreatePartitionAction* cpa)
{
    VolumeTree tree = volumeTrees[ cpa->disk() ];
    DeviceModified* extended = tree.extendedPartition();
    qulonglong newRightBoundary = cpa->offset() + cpa->size();
    
    /* Whether the new partition will be logical is detected automatically computing its boundaries */
    bool isLogical = extended && (cpa->offset() >= extended->offset() && (newRightBoundary <= extended->rightBoundary()));
    
    /* If the new partition won't be logical, check that we're not surpassing the limit of 4 primary partitions */
    if (tree.partitions().count() == 4 && !isLogical) {
        error.setType(PartitioningError::ExceedingPrimariesError);
        error.arg( cpa->disk() );
        return false;
    }
    
    /* A disk can have only one extended partition. */
    if (cpa->partitionType() == Extended && extended) {
        error.setType(PartitioningError::BadPartitionTypeError);
        return false;
    }
    
    return true;
}

bool VolumeManager::Private::gptPartitionChecks(CreatePartitionAction* cpa)
{
    VolumeTree tree = volumeTrees[ cpa->disk() ];
    
    /*
     * This limit should be extendible, but udisks doesn't support it.
     * 
     * FIXME: create a different error type.
     */
    if (tree.partitions().count() == 128) {
        error.setType(PartitioningError::ExceedingPrimariesError);
        error.arg( cpa->disk() );
        return false;
    }
    
    return true;
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
