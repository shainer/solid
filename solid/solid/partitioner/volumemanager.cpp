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
#include <solid/partitioner/volumetreemap.h>
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
    {}
    
    ~Private()
    {}
    
    /* 
    * Apply a new action on the stack. The boolean helps us distinguish between whether the action is being registered by
    * the user or it's been applied by the manager as a result of an undo/redo operation. In the first case, we must push
    * the action on the stack at the end and we also check another identical action hasn't already been pushed before.
    * In the second case, we don't perform this check neither we push an useless duplicate.
    */
    bool applyAction(Action* action, bool isInStack = false);
    
    /*
    * Utility method used from applyAction() to perform some checks on the partition interested by the action,
    * if there is one. 
    */
    bool partitionChecks(Action *);
    
    /* Resize a partition. */
    void resizePartition(VolumeTreeItem *, qulonglong, VolumeTree &);
    
    /* Moves a partition. */
    void movePartition(VolumeTreeItem *, qulonglong, DeviceModified *, VolumeTree &);
    
    /* Perform necessary checks when creating a new partition on either MBR or GPT tables. */
    bool mbrPartitionChecks(Actions::CreatePartitionAction *);
    bool gptPartitionChecks(Actions::CreatePartitionAction *);
    
    /* Set a new ptable scheme for a disk, deleting all the partitions. */
    bool setPartitionTableScheme(const QString &, const QString &);
    
    VolumeTreeMap volumeTreeMap; /* set of trees with disk layouts */
    ActionStack actionstack; /* stack of registered actions */
    PartitioningError error; /* latest error */
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

    d->volumeTreeMap.build();
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

/*
 * TODO: implement
 */
void VolumeManager::undo()
{
    if (!d->actionstack.empty()) {
        d->volumeTreeMap.backToOriginal();
        QList< Action* > undos = d->actionstack.undo();
        
        qDebug() << "UNDO: " << undos.size();
        
        foreach (Action* action, undos) {
            qDebug() << "riapplico" << action->description();
            d->applyAction(action, true);
        }
    }
}

void VolumeManager::redo()
{
    if (!d->actionstack.undoEmpty()) {
        d->volumeTreeMap.backToOriginal();
        Action* newAction = d->actionstack.redo();
        d->applyAction(newAction, true);
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
 * When a partition is added, deletes all the action which took place on the disk (undone ones included).
 * This is because the layout and/or properties have changed so those actions could be out-of-context.
 * Of course when a disk is added no action has to be removed at all.
 */
void VolumeManager::doDeviceAdded(VolumeTree tree)
{
    QString diskName = tree.root()->name();
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
    
    if (!executer.isValid()) {
        d->error.setType(PartitioningError::BusyExecuterError);
        return false;
    }
    
    QObject::connect(&executer, SIGNAL(nextActionCompleted(int)), this, SLOT(doNextActionCompleted(int)));
    bool success = executer.execute();
    
    QObject::disconnect(&executer, SIGNAL(nextActionCompleted(int)), this, SLOT(doNextActionCompleted(int)));
    d->actionstack.clear();
    d->volumeTreeMap.build();
    
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

bool VolumeManager::Private::applyAction(Action* action, bool isInStack)
{    
    if (!partitionChecks(action)) {
        return false;
    }
    
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
            fpa->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::CreatePartition: {
            Actions::CreatePartitionAction* cpa = dynamic_cast< Actions::CreatePartitionAction* >(action);            
            QPair<VolumeTree, DeviceModified* > pair = volumeTreeMap.searchTreeWithDevice( cpa->disk() );
            VolumeTree tree = pair.first;
            
            /* No disk */
            if (!tree.d) {
                error.setType(PartitioningError::DiskNotFoundError);
                error.arg(cpa->disk());
                return false;
            }
            
            Disk* disk = dynamic_cast< Disk* >( pair.second );
            QString scheme = disk->partitionTableScheme();
            
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

            /* Checks whether the partition table supports all the given flags */
            foreach (const QString& flag, cpa->flags()) {
                if (!Utils::PartitionTableUtils::instance()->supportedFlags(scheme).contains(flag)) {
                    error.setType(PartitioningError::PartitionFlagsError);
                    error.arg(flag);
                    return false;
                }
            }
            
            /* Looks for the block of free space that will contain this partition. */
            if (!tree.d->splitCreationContainer(cpa->offset(), cpa->size())) {
                error.setType(PartitioningError::PartitionGeometryError);
                error.arg(QString::number(cpa->offset()));
                error.arg(QString::number(cpa->size()));
                return false;
            }

            Partition* newPartition = new Partition(cpa);
            newPartition->setPartitionTableScheme(scheme);
            newPartition->setFilesystem(cpa->filesystem());
            
            if (cpa->partitionType() == ExtendedPartition) {
                newPartition->setPartitionType(ExtendedPartition);
            }
            
            DeviceModified* extended = tree.extendedPartition();
            if (extended && (cpa->offset() >= extended->offset() && ((cpa->offset() + cpa->size()) <= extended->rightBoundary()))) {
                newPartition->setPartitionType(LogicalPartition);
                newPartition->setParentName(extended->name());
                
                newPartition->setOffset( newPartition->offset() + SPACE_BETWEEN_LOGICALS );
                newPartition->setSize( newPartition->size() - SPACE_BETWEEN_LOGICALS );
            }
            
            tree.d->addDevice(newPartition->parentName(), newPartition);
            cpa->setOwnerDisk(disk);
            break;
        }
        
        case Action::RemovePartition: {
            Actions::RemovePartitionAction* rpa = dynamic_cast< Actions::RemovePartitionAction* >(action);
            
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition( rpa->partition() );
            VolumeTree tree = pair.first;
            Partition* partition = pair.second;
            
            if (!partition) {
                error.setType(PartitioningError::PartitionNotFoundError);
                error.arg( rpa->partition() );
                return false;
            }
            
            if (partition->partitionType() == Utils::ExtendedPartition) {
                foreach (Partition* logical, tree.logicalPartitions()) {
                    if (logical->isMounted()) {
                        error.setType(PartitioningError::MountedLogicalError);
                        return false;
                    }
                }
            }
            
            /* Deletes the partition merging adjacent free blocks if present. */
            tree.d->mergeAndDelete(rpa->partition());
            rpa->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::ResizePartition: {
            Actions::ResizePartitionAction* rpa = dynamic_cast< Actions::ResizePartitionAction* >(action);
            
            VolumeTree tree = volumeTreeMap.searchTreeWithDevice(rpa->partition()).first;            
            VolumeTreeItem* itemToResize = tree.searchNode(rpa->partition());            
            Partition* toResize = dynamic_cast< Partition* >(itemToResize->volume());
            DeviceModified* leftDevice = tree.d->leftDevice(itemToResize);
            DeviceModified* rightDevice = tree.d->rightDevice(itemToResize);
            
            qlonglong oldOffset = (qlonglong)toResize->offset();
            qlonglong oldSize = (qlonglong)toResize->size();
            qlonglong newOffset = (qlonglong)rpa->newOffset();
            qlonglong newSize = (qlonglong)rpa->newSize();
            qlonglong rightBoundary = 0;
            qlonglong leftBoundary = 0;
            
            if (newSize == 0) {
                error.setType(PartitioningError::ResizingToZeroError);
                return false;
            }
            
            if (newOffset == oldOffset && newSize == oldSize) {
                error.setType(PartitioningError::ResizingToTheSameError);
                error.arg(rpa->partition());
                return false;
            }
            
            /* Resizing an extended partition isn't allowed even if the logicals aren't touched. */
            if (toResize->partitionType() == ExtendedPartition) {
                error.setType(PartitioningError::ExtendedResizingError);
                return false;
            }
            
            qlonglong spaceBetween = (toResize->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
            
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
            
            if (newOffset != oldOffset && newOffset < leftBoundary) {
                error.setType(PartitioningError::ResizeOutOfBoundsError);
                return false;
            }
            
            /* If the size is increased beyond the legal boundary... */
            if (newSize != oldSize && (oldOffset + newSize > rightBoundary)) {
                qlonglong difference = oldOffset + newSize - rightBoundary;
                
                /* ... but the offset is reduced of at least that quantity, OK */
                if ((oldOffset - newOffset) < difference) {
                    error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                /*
                 * NOTE: defining an order of the operations isn't really necessary if we're sure everything is legal.
                 * However, avoiding surpassing boundaries even temporarity prevents us from having any issue later.
                 */
                movePartition(itemToResize, newOffset, itemToResize->parent()->volume(), tree);
                resizePartition(itemToResize, newSize, tree);
            }
            /* If the partition is moved forward beyond the legal boundary... */
            else if (newOffset != oldOffset && (newOffset + oldSize >= rightBoundary)) {
                qlonglong difference = newOffset + oldSize - rightBoundary;
                
                /* ... but the size is reduced of at least that quantity, OK */
                if ((oldSize - newSize) < difference) {
                    error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                resizePartition(itemToResize, newSize, tree);
                movePartition(itemToResize, newOffset, itemToResize->parent()->volume(), tree);
            }
            else {
                resizePartition(itemToResize, newSize, tree);
                movePartition(itemToResize, newOffset, itemToResize->parent()->volume(), tree);
            }
            
            rpa->setOwnerDisk(tree.root());
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

            foreach (const QString& flag, mpa->flags()) {
                if (!supportedFlags.contains(flag)) {
                    error.setType(PartitioningError::PartitionFlagsError);
                    error.arg(flag);
                    return false;
                }
            }
            p->setFlags(mpa->flags());
            
            mpa->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::CreatePartitionTable: {
            Actions::CreatePartitionTableAction* cpta = dynamic_cast< Actions::CreatePartitionTableAction* >(action);
            
            if (!setPartitionTableScheme(cpta->disk(), cpta->schemeName())) {
                return false;
            }
            
            VolumeTree tree = volumeTreeMap[cpta->disk()];
            cpta->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::RemovePartitionTable: {
            Actions::RemovePartitionTableAction* rpta = dynamic_cast< Actions::RemovePartitionTableAction* >(action);
            
            if (!setPartitionTableScheme(rpta->disk(), QString())) {
                return false;
            }
            
            VolumeTree tree = volumeTreeMap[rpta->disk()];
            rpta->setOwnerDisk(tree.root());
            break;
        }
        
        default:
            break;
    }
        
    if (!isInStack) {
        actionstack.push(action);
    }
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

void VolumeManager::Private::resizePartition(VolumeTreeItem* itemToResize,
                                             qulonglong newSize,
                                             VolumeTree& tree)
{  
    if (newSize == itemToResize->volume()->size()) {
        return;
    }
    
    Partition* partition = dynamic_cast< Partition* >(itemToResize->volume());
    DeviceModified* rightDevice = tree.d->rightDevice(itemToResize);
    
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
    
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = partition->offset() + newSize;
        qulonglong spaceSize = (rightDevice->offset() - (partition->offset() + newSize) - spaceBetween);
        
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
    partition->setSize(newSize);
}

void VolumeManager::Private::movePartition(VolumeTreeItem* itemToResize,
                                           qulonglong newOffset,
                                           DeviceModified* parent,
                                           VolumeTree& tree)
{
    if (newOffset == itemToResize->volume()->offset()) {
        return;
    }
    
    Partition* partition = dynamic_cast< Partition* >(itemToResize->volume());
    DeviceModified* leftDevice = tree.d->leftDevice(itemToResize);
    DeviceModified* rightDevice = tree.d->rightDevice(itemToResize);
    
    qulonglong oldOffset = partition->offset();
    FreeSpace *freeSpaceRight = 0;
    FreeSpace *freeSpaceLeft = 0;
    qulonglong spaceBetween = (parent->deviceType() == DeviceModified::PartitionDevice) ? SPACE_BETWEEN_LOGICALS : 0;
    
    if (!leftDevice || leftDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        /*
         * Creates a new object for the space left on the left (no pun intended)
         */
        qulonglong spaceOffset = (leftDevice) ? (leftDevice->rightBoundary()) : (parent->offset() + spaceBetween);
        qulonglong spaceSize = newOffset - spaceOffset - spaceBetween;
        
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
    if (cpa->partitionType() == ExtendedPartition && extended) {
        error.setType(PartitioningError::BadPartitionTypeError);
        return false;
    }
    
    return true;
}

bool VolumeManager::Private::gptPartitionChecks(Actions::CreatePartitionAction* cpa)
{
    VolumeTree tree = volumeTreeMap[ cpa->disk() ];
    
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
