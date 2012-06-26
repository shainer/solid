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
#include <solid/partitioner/actions/action.h>
#include <solid/partitioner/actions/formatpartitionaction.h>
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
#include <solid/block.h>
#include <solid/predicate.h>

#include <kglobal.h>

namespace Solid
{   
namespace Partitioner
{
    
using namespace Devices;
using namespace Utils;

class SOLID_EXPORT VolumeManager::Private
{
public:
    Private()
        : newPartitionId(1)
    {}
    
    ~Private()
    {}
    
    /* Applies an action on the disks. */
    bool applyAction(Action *);
    
    /* Performs some standard checks on the partition interested by the action, if any. */
    bool partitionChecks(Action *);
    
    /*
     * Performs a series of check to see if the requested resizing/moving is legal, plus calls the following
     * methods in the right order.
     */
    bool resizeAndMoveSafely(Partition *, qlonglong, qlonglong, VolumeTree &);
    
    /* Resize a partition. */
    void resizePartition(Partition *, DeviceModified *, qulonglong, VolumeTree &);
    
    /* Moves a partition. */
    void movePartition(Partition *, qulonglong, DeviceModified *, DeviceModified *, DeviceModified *, VolumeTree &);
    
    /* Perform necessary checks when creating a new partition on either MBR or GPT tables. */
    bool mbrPartitionChecks(Actions::CreatePartitionAction *);
    bool gptPartitionChecks(Actions::CreatePartitionAction *);
    
    /* Set a new ptable scheme for a disk, deleting all the partitions. */
    bool setPartitionTableScheme(const QString &, const QString &);
    
    VolumeTreeMap volumeTreeMap; /* set of trees with disk layouts */
    ActionStack actionstack; /* stack of registered actions */
    PartitioningError error; /* latest error */
    int newPartitionId; /* incremental ID for new partition's unique names */
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

    d->volumeTreeMap.build(); /* builds all the layout detecting the current status of the hardware */
    
    /* Register this object for receiving notifications about changes in the system */
    QObject::connect(&(d->volumeTreeMap), SIGNAL(deviceAdded(VolumeTree)), this, SLOT(doDeviceAdded(VolumeTree)));
    QObject::connect(&(d->volumeTreeMap), SIGNAL(deviceRemoved(QString,QString)), this, SLOT(doDeviceRemoved(QString,QString)));
}

VolumeManager::~VolumeManager()
{
    d->actionstack.clear();
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
    d->error.setType(PartitioningError::None); /* erase any previous error */
    
    /* A duplicate isn't accepted */
    if (d->actionstack.contains(action)) {
        d->error.setType(PartitioningError::DuplicateActionError);
        d->error.arg(action->description());
        return false;
    }
    
    bool success = d->applyAction(action);
    
    if (success) {
        emit diskChanged(action->ownerDisk()->name());
    }
    
    return success;
}

void VolumeManager::undo()
{
    if (!d->actionstack.empty()) {
        d->volumeTreeMap.backToOriginal();
        QList< Action* > undos = d->actionstack.undo();
        
        foreach (Action* action, undos) {
            d->applyAction(action);
        }
    }
}

void VolumeManager::redo()
{
    if (!d->actionstack.undoEmpty()) {
        d->volumeTreeMap.backToOriginal();
        Action* newAction = d->actionstack.redo();
        d->applyAction(newAction);
    }
}

bool VolumeManager::isUndoPossible() const
{
    return !d->actionstack.empty();
}

bool VolumeManager::isRedoPossible() const
{
    return !d->actionstack.undoEmpty();
}

/*
 * When a partition is added, deletes all the actions which took place on its disk (undone ones included).
 * The actions to deleted are those which have the disk as their owner disk.
 * This is because the disk's layout and/or properties have changed so those actions could be out-of-context.
 * Of course when a disk is added no action has to be removed at all.
 */
void VolumeManager::doDeviceAdded(VolumeTree tree)
{
    QString diskName = tree.disk()->name();
    d->actionstack.removeActionsOfDisk(diskName);
    
    emit deviceAdded(tree);
}

/*
 * See above.
 */
void VolumeManager::doDeviceRemoved(QString udi, QString parentUdi)
{
    d->actionstack.removeActionsOfDisk(parentUdi);
    emit deviceRemoved(udi);
}

VolumeTree VolumeManager::diskTree(const QString& udi) const
{
    return d->volumeTreeMap[udi];
}

QMap< QString, VolumeTree > VolumeManager::allDiskTrees() const
{
    return d->volumeTreeMap.deviceTrees();
}

bool VolumeManager::apply()
{
    ActionExecuter executer( d->actionstack.list(), d->volumeTreeMap );
    
    /*
     * This specific error is triggered when another application is using the executer right now, i.e. owns an instance.
     * This way we avoid conflicts while changing the hardware.
     */
    if (!executer.isValid()) {
        d->error.setType(PartitioningError::BusyExecuterError);
        return false;
    }
    
    QObject::connect(&executer, SIGNAL(nextActionCompleted(int)), this, SLOT(doNextActionCompleted(int)));
    bool success = executer.execute();
    
    QObject::disconnect(&executer, SIGNAL(nextActionCompleted(int)), this, SLOT(doNextActionCompleted(int)));
    d->actionstack.clear();
    d->volumeTreeMap.build(); /* repeats hw detection */
    
    if (!success) {
        d->error = executer.error();
        return false;
    }
    
    return true;
}

void VolumeManager::doNextActionCompleted(int completed)
{
    int actionNumber = d->actionstack.size();
    double progress = (double)completed / (double)actionNumber;
    emit progressChanged(progress);
}

PartitioningError VolumeManager::error() const
{
    return d->error;
}

QList< Action* > VolumeManager::registeredActions() const
{
    return d->actionstack.list();
}

bool VolumeManager::Private::applyAction(Action* action)
{    
    if (!partitionChecks(action)) {
        return false;
    }
    
    /*
     * Each legal action is associated to a "owner disk", i.e. the disk on which said action is performed.
     * See the doDeviceAdded slot for more information on the usefulness of this property.
     */
    Disk* ownerDisk = 0;
    
    switch (action->actionType()) {
        case Action::FormatPartition: {
            Actions::FormatPartitionAction* fpa = dynamic_cast< Actions::FormatPartitionAction* >(action);
            
            QPair<VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition(fpa->partition());
            VolumeTree tree = pair.first;
            Partition* volume = pair.second;
            Utils::Filesystem fs = fpa->filesystem();
            
            /* Other kinds of volume aren't currently supported. Plus, you cannot format an extended partition */
            if ((volume->usage() != StorageVolume::FileSystem && volume->usage() != StorageVolume::Other)
                || (volume->partitionType() == ExtendedPartition)) {
                error.setType(PartitioningError::CannotFormatPartition);
                error.arg( volume->name() );
                return false;
            }

            /* At least one flag of the specified filesystem is out-of-context */
            if (!fs.unsupportedFlags().isEmpty()) {
                error.setType(PartitioningError::FilesystemFlagsError);
                error.arg(fpa->filesystem().unsupportedFlags().join(", "));
                return false;
            }
            
            volume->setFilesystem(fs);
            ownerDisk = tree.disk();
            break;
        }
        
        case Action::CreatePartition: {
            Actions::CreatePartitionAction* cpa = dynamic_cast< Actions::CreatePartitionAction* >(action);            
            VolumeTree tree = volumeTreeMap.searchTreeWithDevice( cpa->disk() ).first;
            
            /* No disk was found */
            if (!tree.d) {
                error.setType(PartitioningError::DiskNotFoundError);
                error.arg(cpa->disk());
                return false;
            }
            
            Disk* disk = tree.disk();
            QString scheme = disk->partitionTableScheme();
            
            /* Check if everything is ok with the current scheme. */
            if (scheme.isEmpty()) {
                error.setType(PartitioningError::NoPartitionTableError);
                error.arg( cpa->disk() );
                return false;
            }
            else if (scheme == "mbr" && !mbrPartitionChecks(cpa)) {
                return false;
            }
            else if (scheme == "gpt" && !gptPartitionChecks(cpa)) {
                return false;
            }
            
            if (cpa->size() < disk->minimumPartitionSize()) {
                error.setType(PartitioningError::PartitionTooSmallError);
                return false;
            }

            /* Checks whether the partition table supports all the given flags */
            foreach (const QString& flag, cpa->flags()) {
                if (!Utils::PartitionTableUtils::instance()->supportedFlags(scheme).contains(flag)) {
                    error.setType(PartitioningError::PartitionFlagsError);
                    error.arg(flag);
                    return false;
                }
            }
            
            /* Looks for the block of free space that will contain this partition, if present. */
            if (!tree.d->splitCreationContainer(cpa->offset(), cpa->size())) {
                error.setType(PartitioningError::PartitionGeometryError);
                error.arg(QString::number(cpa->offset()));
                error.arg(QString::number(cpa->size()));
                return false;
            }
            
            cpa->setPartitionName( "New partition " + QString::number(newPartitionId++) );
            Partition* newPartition = new Partition(cpa);
            newPartition->setPartitionTableScheme(scheme);
            newPartition->setFilesystem( cpa->filesystem() );
            newPartition->setPartitionType( cpa->partitionType() );
            
            if (newPartition->partitionType() == LogicalPartition) {
                newPartition->setParentName(tree.extendedPartition()->name());
                
                /*
                 * This leaves some space for the EBR. It's not strictly necessary as udisks reserves it anyway, but
                 * it keeps the layout cleaner and simplifies future actions.
                 */
                newPartition->setOffset( newPartition->offset() + SPACE_BETWEEN_LOGICALS );
                newPartition->setSize( newPartition->size() - SPACE_BETWEEN_LOGICALS );
            }
            
            tree.d->addDevice(newPartition->parentName(), newPartition);
            ownerDisk = disk;
            break;
        }
        
        case Action::RemovePartition: {
            Actions::RemovePartitionAction* rpa = dynamic_cast< Actions::RemovePartitionAction* >(action);
            
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition( rpa->partition() );
            VolumeTree tree = pair.first;
            Partition* partition = pair.second;
            
            /*
             * Removing an extended partition removes all the logicals too. So we have to deny the action if
             * at least one of the logical partitions is currently mounted.
             */
            if (partition->partitionType() == ExtendedPartition) {
                foreach (Partition* logical, tree.logicalPartitions()) {
                    if (logical->isMounted()) {
                        error.setType(PartitioningError::MountedLogicalError);
                        return false;
                    }
                }
            }
            
            /* Deletes the partition merging adjacent free blocks if present. */
            tree.d->mergeAndDelete(rpa->partition());
            ownerDisk = tree.disk();
            break;
        }
        
        case Action::ResizePartition: {
            Actions::ResizePartitionAction* rpa = dynamic_cast< Actions::ResizePartitionAction* >(action);
            
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition( rpa->partition() );
            VolumeTree tree = pair.first;           
            Partition* toResize = pair.second;
            Disk* disk = tree.disk();
            
            qlonglong oldOffset = (qlonglong)toResize->offset();
            qlonglong oldSize = (qlonglong)toResize->size();
            qlonglong newOffset = (qlonglong)rpa->newOffset();
            qlonglong newSize = (qlonglong)rpa->newSize();
            
            if (newSize == 0) {
                error.setType(PartitioningError::ResizingToZeroError);
                return false;
            }
            
            if (newOffset == oldOffset && newSize == oldSize) {
                error.setType(PartitioningError::ResizingToTheSameError);
                error.arg(rpa->partition());
                return false;
            }
            
            /* The minimum partition size constraint has to be respected here too */
            if (newSize < disk->minimumPartitionSize()) {
                error.setType(PartitioningError::PartitionTooSmallError);
                return false;
            }
            
            /* Resizing an extended partition isn't allowed even if the logicals aren't touched. */
            if (toResize->partitionType() == ExtendedPartition) {
                error.setType(PartitioningError::ExtendedResizingError);
                return false;
            }
            
            /* Performs all the "geometry" checks and applies the action if it's legal */
            if (!resizeAndMoveSafely(toResize, newOffset, newSize, tree)) {
                return false;
            }
            
            ownerDisk = disk;
            break;
        }
        
        case Action::ModifyPartition: {
            Actions::ModifyPartitionAction* mpa = dynamic_cast< Actions::ModifyPartitionAction* >(action);
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition( mpa->partition() );
            VolumeTree tree = pair.first;
            Partition* p = pair.second;
            
            if (mpa->isLabelChanged()) {
                p->setLabel( mpa->label() );
            }

            QStringList supportedFlags = PartitionTableUtils::instance()->supportedFlags( p->partitionTableScheme() );

            /* Check if the action sets supported flags */
            foreach (const QString& flag, mpa->flags()) {
                if (!supportedFlags.contains(flag)) {
                    error.setType(PartitioningError::PartitionFlagsError);
                    error.arg(flag);
                    return false;
                }
            }
            p->setFlags(mpa->flags());
            
            ownerDisk = tree.disk();
            break;
        }
        
        case Action::CreatePartitionTable: {
            Actions::CreatePartitionTableAction* cpta = dynamic_cast< Actions::CreatePartitionTableAction* >(action);
            
            if (!setPartitionTableScheme(cpta->disk(), cpta->schemeName())) {
                return false;
            }
            
            VolumeTree tree = volumeTreeMap[cpta->disk()];
            ownerDisk = tree.disk();
            break;
        }
        
        case Action::RemovePartitionTable: {
            Actions::RemovePartitionTableAction* rpta = dynamic_cast< Actions::RemovePartitionTableAction* >(action);
            
            if (!setPartitionTableScheme(rpta->disk(), QString())) {
                return false;
            }
            
            VolumeTree tree = volumeTreeMap[rpta->disk()];
            ownerDisk = tree.disk();
            break;
        }
        
        default:
            break;
    }

    action->setOwnerDisk(ownerDisk);
    actionstack.push(action);
    return true;
}

bool VolumeManager::Private::partitionChecks(Action* a)
{
    if (Utils::isPartitionAction(a)) {
        PartitionAction* action = dynamic_cast< PartitionAction* >(a);
        Partition* partition = volumeTreeMap.searchPartition( action->partition() );
        
        if (!partition) {
            error.setType(PartitioningError::PartitionNotFoundError);
            error.arg( action->partition() );
            return false;
        }
        
        if (partition->isMounted()) {
            error.setType(PartitioningError::MountedPartitionError);
            error.arg( partition->name() );
            return false;
        }
    }
    
    return true; /* no need to check anything for those actions not concerning partitions */
}

/*
 * NOTE: while the meaning of changing the size is obvious, an explanation is needed for the offset.
 * In this module, changing the offset means *moving* the partition backward or forward, while the size stays the same.
 */
bool VolumeManager::Private::resizeAndMoveSafely(Partition* toResize,
                                                 qlonglong newOffset,
                                                 qlonglong newSize,
                                                 VolumeTree& disk)
{
    DeviceModified* rightDevice = disk.d->rightDevice(toResize);
    DeviceModified* leftDevice = disk.d->leftDevice(toResize);
    DeviceModified* parent = disk.searchNode(toResize->name())->parent()->volume();
    
    qlonglong oldOffset = (qlonglong)(toResize->offset());
    qlonglong oldSize = (qlonglong)(toResize->size());
    qlonglong spaceBetween = (toResize->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
    qlonglong rightBoundary = 0, leftBoundary = 0;
    
    /* Finds what's the biggest right boundary this partition can have */
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        rightBoundary = toResize->rightBoundary();
    } else {
        rightBoundary = rightDevice->rightBoundary();
    }
    
    /* Finds what's the minimum offset this partition can have */
    if (!leftDevice || leftDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        leftBoundary = toResize->offset();
    } else {
        leftBoundary = leftDevice->offset() + spaceBetween;
    }
    
    if (newOffset < leftBoundary) {
        error.setType(PartitioningError::ResizeOutOfBoundsError);
        return false;
    }
    
    /* If the size is increased beyond the legal boundary... */
    if (oldOffset + newSize > rightBoundary) {
        qlonglong difference = oldOffset + newSize - rightBoundary;
        
        /* ... but the offset is reduced of at least that quantity, OK */
        if ((oldOffset - newOffset) < difference) {
            error.setType(PartitioningError::ResizeOutOfBoundsError);
            return false;
        }
        
       /*
        * NOTE: defining an order of the operations isn't really necessary if we're sure everything is legal.
        * However, avoiding surpassing boundaries even temporarity prevents us from having hidden bugs and sign problems.
        */
        movePartition(toResize, newOffset, parent, leftDevice, rightDevice, disk);
        resizePartition(toResize, rightDevice, newSize, disk);
    }
    /* If the partition is moved forward beyond the legal boundary... */
    else if (newOffset != oldOffset && (newOffset + oldSize >= rightBoundary)) {
        qlonglong difference = newOffset + oldSize - rightBoundary;
        
        /* ... but the size is reduced of at least that quantity, OK */
        if ((oldSize - newSize) < difference) {
            error.setType(PartitioningError::ResizeOutOfBoundsError);
            return false;
        }
        
        resizePartition(toResize, rightDevice, newSize, disk);
        movePartition(toResize, newOffset, parent, leftDevice, rightDevice, disk);
    }
    else { /* no specific order needed */
        resizePartition(toResize, rightDevice, newSize, disk);
        movePartition(toResize, newOffset, parent, leftDevice, rightDevice, disk);
    }
    
    return true;
}

void VolumeManager::Private::resizePartition(Partition* partition,
                                             DeviceModified* rightDevice,
                                             qulonglong newSize,
                                             VolumeTree& disk)
{  
    if (newSize == partition->size()) {
        return;
    }
    
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
    
    /* Creates a new object for the free space left on the right */
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qlonglong spaceOffset = partition->offset() + newSize;
        qlonglong spaceSize = (rightDevice->offset() - (partition->offset() + newSize) - spaceBetween);
        
        if (spaceSize > 0) {
            FreeSpace* freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
            disk.d->addDevice(partition->parentName(), freeSpaceRight);
        }
    } else {
        FreeSpace* spaceRight = dynamic_cast< FreeSpace* >(rightDevice);
        qlonglong rightOffset = partition->offset() + newSize;
        qlonglong rightSize = spaceRight->size() - (newSize - partition->size());
                        
        if (rightSize == 0) { /* we filled up the space available */
            disk.d->removeDevice(spaceRight->name());
        } else {
            spaceRight->setOffset(rightOffset);
            spaceRight->setSize(rightSize);
        }
    }
    partition->setSize(newSize);
}

void VolumeManager::Private::movePartition(Partition* partition,
                                           qulonglong newOffset,
                                           DeviceModified* parent,
                                           DeviceModified* leftDevice,
                                           DeviceModified* rightDevice,
                                           VolumeTree& tree)
{
    if (newOffset == partition->offset()) {
        return;
    }
    
    qulonglong oldOffset = partition->offset();
    FreeSpace *freeSpaceRight = 0;
    FreeSpace *freeSpaceLeft = 0;
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
    
    /* Creates a new object for the space left on the left */
    if (!leftDevice || leftDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = (leftDevice) ? (leftDevice->rightBoundary()) : (parent->offset() + spaceBetween);
        qulonglong spaceSize = newOffset - spaceOffset - spaceBetween;
        
        if (spaceSize > 0) {
            freeSpaceLeft = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
        }
    } else { /* there's some free space immediately before: changes its size accordingly */
        qulonglong leftSize = leftDevice->size() - (oldOffset - newOffset);

        if (leftSize == 0) { /* we filled up all the space */
            tree.d->removeDevice(leftDevice->name());
        } else {
            leftDevice->setSize(leftSize);
        }
    }
    
    /*
     * Moving a partition around affects what's on the right too.
     * NOTE for future developers: we can't use resizePartition for this because in this case
     * the size isn't changing, just the position of the partition.
     */
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = newOffset + partition->size();
        qulonglong spaceSize = (rightDevice->offset() - (newOffset + partition->size()) - spaceBetween);
                        
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
    VolumeTree tree = volumeTreeMap[cpa->disk()];
    DeviceModified* extended = tree.extendedPartition();
    
    /* Whether the new partition will be logical is detected automatically looking at its boundaries */
    bool isLogical = cpa->partitionType() == LogicalPartition;
    
    /* If the new partition won't be logical, check that we're not surpassing the limit of 4 primary partitions */
    if (tree.partitions().count() == 4 && !isLogical) {
        error.setType(PartitioningError::ExceedingPrimariesError);
        error.arg( cpa->disk() );
        return false;
    }
    
    /* A disk can have only one extended partition. */
    if (cpa->partitionType() == ExtendedPartition && extended) {
        error.setType(PartitioningError::BadPartitionTypeError);
        return false;
    }
    
    return true;
}

bool VolumeManager::Private::gptPartitionChecks(Actions::CreatePartitionAction* cpa)
{
    VolumeTree tree = volumeTreeMap[ cpa->disk() ];
    
    /* This limit should be extendible, but udisks doesn't support it */
    if (tree.partitions().count() == 128) {
        error.setType(PartitioningError::ExceedingGPTPartitionsError);
        error.arg( cpa->disk() );
        return false;
    }
    
    return true;
}

bool VolumeManager::Private::setPartitionTableScheme(const QString& diskName, const QString& scheme)
{
    VolumeTree tree = volumeTreeMap[diskName];

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

}
}
