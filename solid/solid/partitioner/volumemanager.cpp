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

class VolumeManager::Private
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
    
    /* Resize a partition. */
    void resizePartition(Partition *, qlonglong, DeviceModified *, VolumeTree &);
    
    /* Moves a partition. */
    void movePartition(Partition *, qlonglong, DeviceModified *, DeviceModified *, DeviceModified *, VolumeTree &);
    
    /* Perform necessary checks when creating a new partition on either MBR or GPT tables. */
    bool mbrPartitionChecks(Actions::CreatePartitionAction *);
    bool gptPartitionChecks(Actions::CreatePartitionAction *);
    
    /* Set a new ptable scheme for a disk, deleting all the partitions. */
    bool setPartitionTableScheme(const QString &, Utils::PartitionTableScheme);
    
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

/*
 * FIXME: less duplicate code
 */
bool VolumeManager::Private::applyAction(Action* action, bool isInStack)
{    
    switch (action->actionType()) {
        case Action::FormatPartition: {
            Actions::FormatPartitionAction* fpa = dynamic_cast< Actions::FormatPartitionAction* >(action);
            
            QPair<VolumeTree, DeviceModified* > pair = volumeTreeMap.searchTreeWithDevice(fpa->partition());
            VolumeTree tree = pair.first;
            DeviceModified* p = pair.second;
            Utils::Filesystem fs = fpa->filesystem();

            /* At least one flag of the specified filesystem is out-of-context */
            if (!fs.unsupportedFlags().isEmpty()) {
                error.setType(PartitioningError::FilesystemFlagsError);
                error.arg(fpa->filesystem().unsupportedFlags().join(", "));
                return false;
            }
            
            /* Unexistent device */
            if (!p) {
                error.setType(PartitioningError::PartitionNotFoundError);
                error.arg(fpa->partition());
                return false;
            }
            
            /* The specified device isn't a partition. */
            if (p->deviceType() != DeviceModified::PartitionDevice) {
                error.setType(PartitioningError::WrongDeviceTypeError);
                error.arg("partition");
                return false;
            }
            
            Partition* volume = dynamic_cast<Partition *>(p);
            
            if (volume->usage() != StorageVolume::FileSystem) {
                error.setType(PartitioningError::CannotFormatPartition);
                error.arg(volume->name());
                return false;
            }
            
            if (volume->isMounted()) {
                error.setType(PartitioningError::MountedPartitionError);
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
            Utils::PartitionTableScheme scheme = disk->partitionTableScheme();

            switch (scheme) {
                case Utils::NoneScheme: {
                    error.setType(PartitioningError::NoPartitionTableError);
                    error.arg( cpa->disk() );
                    return false;
                }
                
                case Utils::MBRScheme: {
                    if (!mbrPartitionChecks(cpa)) {
                        return false;
                    }
                    
                    break;
                }
                
                case Utils::GPTScheme: {
                    if (!gptPartitionChecks(cpa)) {
                        return false;
                    }
                    
                    break;
                }
                
                default:
                    break;
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
                error.setType(PartitioningError::ContainerNotFoundError);
                error.arg(QString::number(cpa->offset()));
                error.arg(QString::number(cpa->size()));
                return false;
            }

            Partition* newPartition = new Partition(cpa);
            newPartition->setFilesystem(cpa->filesystem());
            tree.d->addDevice(cpa->disk(), newPartition);
            
            cpa->setOwnerDisk(disk);
            break;
        }
        
        case Action::RemovePartition: {
            Actions::RemovePartitionAction* rpa = dynamic_cast< Actions::RemovePartitionAction* >(action);
            
            QPair< VolumeTree, DeviceModified* > pair = volumeTreeMap.searchTreeWithDevice( rpa->partition() );
            VolumeTree tree = pair.first;
            DeviceModified* dev = pair.second;

            if (!tree.d) {
                error.setType(PartitioningError::PartitionNotFoundError);
                error.arg(rpa->partition());
                return false;
            }
            
            if (dev->deviceType() != DeviceModified::PartitionDevice) {
                error.setType(PartitioningError::WrongDeviceTypeError);
                error.arg("partition");
            }
            
            Partition* p = dynamic_cast< Partition* >(dev);
            if (p->isMounted()) {
                error.setType(PartitioningError::MountedPartitionError);
                return false;
            }
            
            /* Deletes the partition merging adjacent free blocks if present. */
            tree.d->mergeAndDelete(rpa->partition());
            rpa->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::ResizePartition: {
            Actions::ResizePartitionAction* rpa = dynamic_cast< Actions::ResizePartitionAction* >(action);
            
            VolumeTree tree = volumeTreeMap.searchTreeWithDevice(rpa->partition()).first;
            
            if (!tree.d) {
                error.setType(PartitioningError::PartitionNotFoundError);
                error.arg(rpa->partition());
                return false;
            }
            
            VolumeTreeItem* itemToResize = tree.searchNode(rpa->partition());            
            Partition* toResize = dynamic_cast< Partition* >(itemToResize->volume());
            DeviceModified* leftDevice = tree.d->leftDevice(itemToResize);
            DeviceModified* rightDevice = tree.d->rightDevice(itemToResize);
            
            qlonglong boundary = 0;
            
            if (toResize->isMounted()) {
                error.setType(PartitioningError::MountedPartitionError);
                return false;
            }
            
            if (rpa->newSize() == 0) {
                error.setType(PartitioningError::ResizingToZeroError);
                return false;
            }
            
            if (rpa->newSize() == -1 && rpa->newOffset() == -1) {
                error.setType(PartitioningError::ResizingToTheSameError);
                error.arg(rpa->partition());
                return false;
            }
            
            /* The offset is moved to the left, but there is no free space for such move */
            if (leftDevice && leftDevice->deviceType() != DeviceModified::FreeSpaceDevice && rpa->newOffset() < toResize->offset()) {
                error.setType(PartitioningError::ResizeOutOfBoundsError);
                return false;
            }
    
            /*
             * For extended partitions it avoids "crushing" any logical one.
             */
            DeviceModified* firstLogical = 0;
            DeviceModified* lastLogical = 0;
            if (toResize->partitionType() == ExtendedPartition) {
                lastLogical = tree.extendedNode()->children().last()->volume();
                firstLogical = tree.extendedNode()->children().first()->volume();
                
                if (rpa->newSize() < toResize->size() &&
                   (lastLogical->deviceType() != DeviceModified::FreeSpaceDevice ||
                    toResize->offset() + rpa->newSize() < lastLogical->offset())) {
                    error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                if (rpa->newOffset() > toResize->offset() &&
                    (firstLogical->deviceType() != DeviceModified::FreeSpaceDevice ||
                    toResize->offset() > firstLogical->rightBoundary())) {
                    error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
            }
            
            /* Finds what's the biggest right boundary the new partition can have */
            if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
                boundary = toResize->rightBoundary();
            } else {
                boundary = rightDevice->offset() + rightDevice->size();
            }
            
            /* If the size is increased beyond the legal boundary... */
            if ((toResize->offset() + rpa->newSize()) > boundary) {
                qlonglong difference = toResize->offset() + rpa->newSize() - boundary;
                
                /* ... but the offset is reduced of at least that quantity, OK */
                if ((toResize->offset() - rpa->newOffset()) < difference) {
                    error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                /*
                 * NOTE: defining an order of the operations isn't really necessary if we're sure everything is legal.
                 * However, avoiding surpassing boundaries even temporarity prevents us from having any issue later.
                 */
                movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
                resizePartition(toResize, rpa->newSize(), rightDevice, tree);
            }
            /* If the partition is moved forward beyond the legal boundary... */
            else if (rpa->newOffset() + toResize->size() >= boundary) {
                qlonglong difference = rpa->newOffset() + toResize->size() - boundary;

                /* ... but the size is reduced of at least that quantity, OK */
                if ((toResize->size() - rpa->newSize()) < difference) {
                    error.setType(PartitioningError::ResizeOutOfBoundsError);
                    return false;
                }
                
                resizePartition(toResize, rpa->newSize(), rightDevice, tree);
                movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
            }
            else {
                resizePartition(toResize, rpa->newSize(), rightDevice, tree);
                movePartition(toResize, rpa->newOffset(), leftDevice, rightDevice, itemToResize->parent()->volume(), tree);
            }
            
            rpa->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::ModifyPartition: {
            Actions::ModifyPartitionAction* mpa = dynamic_cast< Actions::ModifyPartitionAction* >(action);
            QPair< VolumeTree, DeviceModified* > pair = volumeTreeMap.searchTreeWithDevice( mpa->partition() );
            DeviceModified* device = pair.second;
            VolumeTree tree = pair.first;
            
            if (!device) {
                error.setType(PartitioningError::PartitionNotFoundError);
                error.arg(mpa->partition());
                return false;
            }
            
            if (device->deviceType() != DeviceModified::PartitionDevice) {
                error.setType(PartitioningError::WrongDeviceTypeError);
                error.arg("partition");
                return false;
            }
            
            Partition* p = dynamic_cast< Partition* >(device);
            
            if (p->isMounted()) {
                error.setType(PartitioningError::MountedPartitionError);
                return false;
            }
            
            if (mpa->isLabelChanged()) {
                p->setLabel( mpa->label() );
            }

            Disk* parent = dynamic_cast< Disk* >( tree.searchNode( mpa->partition() )->parent()->volume() );
            QStringList supportedFlags = PartitionTableUtils::instance()->supportedFlags( parent->partitionTableScheme() );

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
            
            if (!setPartitionTableScheme(cpta->disk(), cpta->partitionTableScheme())) {
                return false;
            }
            
            VolumeTree tree = volumeTreeMap[cpta->disk()];
            cpta->setOwnerDisk(tree.root());
            break;
        }
        
        case Action::RemovePartitionTable: {
            Actions::RemovePartitionTableAction* rpta = dynamic_cast< Actions::RemovePartitionTableAction* >(action);
            
            if (!setPartitionTableScheme(rpta->disk(), Utils::NoneScheme)) {
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

void VolumeManager::Private::resizePartition(Partition* partition,
                                    qlonglong ns,
                                    DeviceModified* rightDevice,
                                    VolumeTree& tree)
{  
    if (ns == -1 || ns == partition->size()) {
        return;
    }
    qulonglong newSize = (qulonglong)ns;
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition ? SPACE_BETWEEN_LOGICALS : 0);
    
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
    
    if (partition->partitionType() == ExtendedPartition) {
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
    
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition ? SPACE_BETWEEN_LOGICALS : 0);
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
    
    if (partition->partitionType() == ExtendedPartition) {
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

bool VolumeManager::Private::setPartitionTableScheme(const QString& diskName, PartitionTableScheme scheme)
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
