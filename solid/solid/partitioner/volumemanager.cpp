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
#include <solid/partitioner/actions/modifyfilesystemaction.h>
#include <solid/partitioner/partitioningerror.h>
#include <solid/partitioner/utils/utils.h>
#include <solid/partitioner/utils/filesystemutils.h>
#include <solid/partitioner/utils/deviceevent.h>
#include <solid/partitioner/volumetreemap_p.h>
#include <solid/device.h>
#include <solid/block.h>
#include <solid/predicate.h>
#include <backends/udisks/udisksmanager.h>

#include <kglobal.h>
#include <stdlib.h>

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
        , deviceManager( new Backends::UDisks::UDisksManager(0) )
        , acceptEvents(true)
    {}
    
    ~Private()
    {}
    
    /*
     * Applies an action on the disks.
     * 
     * All the new registered actions are pushed in the stack. The parameter is set to true by undo() and redo()
     * to avoid this, since the interested action is already present on the stack.
     */
    bool applyAction(Action *, bool undoOrRedo = false);
    
    /* Performs some standard checks on the partition interested by the action, if any. */
    bool partitionChecks(Action *);
    
    /*
     * Performs some checks on the filesystem to be added to a partition. Returns true if all tests have passed.
     * Otherwise, it automatically sets the right error type.
     */
    bool filesystemChecks(const Filesystem &);
    
    /*
     * Performs some standard checks on a partition's label.
     * Parameters: the partition table scheme and the label.
     */
    bool labelChecks(const QString &, const QString &);
    
    /*
     * Performs a series of check to see if the requested resizing/moving is legal, plus calls the following
     * methods in the right order.
     */
    bool resizeAndMoveSafely(Partition *, qlonglong, qlonglong, VolumeTree &);
    
    /* Resize a partition. */
    void resizePartition(Partition *, qulonglong, VolumeTree &);
    
    /* Moves a partition. */
    void movePartition(Partition *, qulonglong, DeviceModified *, VolumeTree &);
    
    /* Perform necessary checks when creating a new partition on either MBR or GPT tables. */
    bool mbrPartitionChecks(Actions::CreatePartitionAction *);
    bool gptPartitionChecks(Actions::CreatePartitionAction *);
    
    /* Set a new ptable scheme for a disk, deleting all the partitions. */
    bool setPartitionTableScheme(const QString &, const QString &);
    
    VolumeTreeMap volumeTreeMap; /* set of trees with disk layouts */
    ActionStack actionstack; /* stack of registered actions */
    PartitioningError error; /* latest error */
    int newPartitionId; /* incremental ID for new partition's unique names */
    Ifaces::DeviceManager* deviceManager;
    bool acceptEvents; /* whether the manager processes deviceAdded and deviceRemoved signals. */
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
        
    d->volumeTreeMap.d->build(); /* builds all the layout detecting the current status of the hardware */
    
    QObject::connect(d->deviceManager, SIGNAL(deviceAdded(QString)), SLOT(doDeviceAdded(QString)));
    QObject::connect(d->deviceManager, SIGNAL(deviceRemoved(QString)), SLOT(doDeviceRemoved(QString)));
    
    /*
     * Acquire a list of all the partitions to notify whether one of them is mounted or umounted.
     */
    QList< Partition* > partitions;
    foreach (const VolumeTree& diskTree, d->volumeTreeMap.deviceTrees()) {
        partitions += diskTree.partitions();
        partitions += diskTree.logicalPartitions();
    }
    
    foreach (Partition* partition, partitions) {
        if (partition->access()) {
            QObject::connect(partition->access(),
                            SIGNAL(accessibilityChanged(bool, const QString &)),
                            SIGNAL(accessibilityChanged(bool, const QString &)));
        }
    }
}

VolumeManager::~VolumeManager()
{
    d->deviceManager->deleteLater();
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
    Q_ASSERT(action);
    d->error.setType(PartitioningError::None); /* erase any previous error */
    
    /* A duplicate isn't accepted */
    if (d->actionstack.contains(action)) {
        d->error.setType(PartitioningError::DuplicateActionError);
        d->error.arg( action->description() );
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
    d->error.setType(PartitioningError::None);
    
    if (isUndoPossible()) {
        /*
         * Retrieve now the owner disk of the undone action, as both the list returned by undo() and the registered
         * action list may be empty later.
         */
        DeviceModified* ownerDisk = d->actionstack.list().last()->ownerDisk();
        d->volumeTreeMap.d->backToOriginal();

        foreach (Action* action, d->actionstack.undo()) {
            d->applyAction(action, true); /* don't put on stack as they are already there */
        }
        
        emit diskChanged( ownerDisk->name() );
    }
}

void VolumeManager::redo()
{
    d->error.setType(PartitioningError::None);
    
    if (isRedoPossible()) {
        Action* newAction = d->actionstack.redo();
        d->applyAction(newAction, true);
        
        emit diskChanged( newAction->ownerDisk()->name() );
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
void VolumeManager::doDeviceAdded(QString udi)
{
    if (!d->acceptEvents) {
        return;
    }
    
    d->volumeTreeMap.d->addDevice(udi);
    QPair< VolumeTree, DeviceModified* > pair = d->volumeTreeMap.searchTreeWithDevice(udi);
    VolumeTree tree = pair.first;
    DeviceModified* dev = pair.second;
    
    QString diskName = tree.disk()->name();
    d->actionstack.removeActionsOfDisk(diskName);
    
    /*
     * For a partition, enable notifies of changes in its accessibility status.
     */
    if (dev->deviceType() == DeviceModified::PartitionDevice) {
        Partition* p = dynamic_cast< Partition* >(dev);
        
        if (p->access()) {
            QObject::connect(p->access(),
                            SIGNAL(accessibilityChanged(bool, const QString &)),
                            SIGNAL(accessibilityChanged(bool, const QString &)));
        }
    }
    
    emit deviceAdded(tree);
}

/*
 * See above.
 */
void VolumeManager::doDeviceRemoved(QString udi)
{
    if (!d->acceptEvents) {
        return;
    }
    
    d->volumeTreeMap.d->removeDevice(udi);
    d->actionstack.removeActionsOfDisk(udi);
    emit deviceRemoved(udi);
}

VolumeTree VolumeManager::diskTree(const QString& udi) const
{
    return d->volumeTreeMap[udi];
}

VolumeTreeMap VolumeManager::allDiskTrees() const
{
    return d->volumeTreeMap;
}

bool VolumeManager::apply()
{
    d->acceptEvents = false;
    d->error.setType(PartitioningError::None);
    ActionExecuter executer( d->actionstack.list(), d->volumeTreeMap );
    
    /*
     * This specific error is triggered when another application is using the executer right now, i.e. owns an instance.
     * This way we avoid conflicts while changing the hardware.
     */
    if (!executer.isValid()) {
        d->error.setType(PartitioningError::BusyExecuterError);
        emit executionError( d->error.description() );
        return false;
    }
    
    QObject::connect(&executer, SIGNAL(nextActionCompleted(int)), this, SLOT(doNextActionCompleted(int)), Qt::DirectConnection);
    bool success = executer.execute();
    QObject::disconnect(&executer, SIGNAL(nextActionCompleted(int)), this, SLOT(doNextActionCompleted(int)));
    
    d->volumeTreeMap.d->build(); /* repeats hw detection */
    d->actionstack.clear();
    
    if (!success) {
        d->error = executer.error();
        emit executionError( d->error.description() );
        return false;
    }
    
    emit executionFinished();
    d->acceptEvents = true;
    return true;
}

void VolumeManager::doNextActionCompleted(int completed)
{
    emit progressChanged(completed);
}

PartitioningError VolumeManager::error() const
{
    return d->error;
}

QList< Action* > VolumeManager::registeredActions() const
{
    return d->actionstack.list();
}

bool VolumeManager::Private::applyAction(Action* action, bool undoOrRedo)
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
            
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition(fpa->partition());
            VolumeTree tree = pair.first;
            Partition* volume = pair.second;
            Filesystem fs = fpa->filesystem();
            
            /* Other kinds of volume aren't currently supported. Plus, you cannot format an extended partition */
            if ((volume->usage() != StorageVolume::FileSystem && volume->usage() != StorageVolume::Other)
                || (volume->partitionType() == ExtendedPartition)) {
                error.setType(PartitioningError::CannotFormatPartition);
                error.arg( volume->name() );
                return false;
            }
            
            if (!filesystemChecks(fs)) {
                return false;
            }
            
            if (volume->size() < FilesystemUtils::instance()->minimumFilesystemSize( fs.name() )) {
                error.setType(PartitioningError::FilesystemMinSizeError);
                error.arg( fs.name() );
                return false;
            }
            
            Filesystem oldFs = volume->filesystem();
            volume->setFilesystem(fs);
            ownerDisk = tree.disk();
            break;
        }
        
        case Action::ModifyFilesystem: {
            Actions::ModifyFilesystemAction* mfa = dynamic_cast< ModifyFilesystemAction* >(action); 
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition( mfa->partition() );
            Partition* partition = pair.second;
            Filesystem filesystem = partition->filesystem();
            
            if (!FilesystemUtils::instance()->supportsLabel( filesystem.name() ) ||
                mfa->fsLabel().size() > FilesystemUtils::instance()->filesystemProperty( filesystem.name(), "max_label_len" ).toInt() ) {
                error.setType(PartitioningError::FilesystemLabelError);
                error.arg( filesystem.name() );
                return false;
            }
            
            QString oldLabel = partition->label();
            filesystem.setLabel( mfa->fsLabel() );
            partition->setFilesystem(filesystem);
            
            /*
             * This requires some explanation. In DOS tables, labeled partitions aren't supported. So technically the label
             * property of the Partition class is only meaningful when GPT is used. However, UDisks adds some ambiguity by
             * considering the filesystem label as the partition label in MBR (if you call partition->label() in MBR, that's
             * what you get, instead of a default value). So we go on with this behaviour here to avoid messing up things.
             * 
             * This is also (I hope) easier for applications so they can just display the partition label and be happy.
             */
            if (partition->partitionTableScheme() == "mbr") {
                partition->setLabel( mfa->fsLabel() );
            }
            
            ownerDisk = pair.first.disk();
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
            
            if (!labelChecks(scheme, cpa->label())) {
                return false;
            }
            
            if (!filesystemChecks( cpa->filesystem() )) {
                return false;
            }
            
            if (cpa->size() < FilesystemUtils::instance()->minimumFilesystemSize( cpa->filesystem().name() )) {
                error.setType(PartitioningError::FilesystemMinSizeError);
                error.arg( cpa->filesystem().name() );
                return false;
            }
            
            /* Looks for the block of free space that will contain this partition, if present. */
            if (!tree.d->splitCreationContainer(cpa->offset(), cpa->size())) {
                error.setType(PartitioningError::PartitionGeometryError);
                error.arg(QString::number(cpa->offset()));
                error.arg(QString::number(cpa->size()));
                return false;
            }
            
            cpa->setPartitionName( "New partition " + QString::number(newPartitionId++) );
            Partition* newPartition = new Partition(cpa, scheme);
            
            if (newPartition->partitionType() == LogicalPartition) {
                newPartition->setParentName(tree.extendedPartition()->name());
                
                /*
                 * This leaves some space for the EBR. It's not strictly necessary as udisks reserves it anyway, but
                 * it keeps the layout cleaner and simplifies future actions.
                 */
                newPartition->setOffset( newPartition->offset() + SPACE_BETWEEN_LOGICALS );
                newPartition->setSize( newPartition->size() - SPACE_BETWEEN_LOGICALS );
            }
            
            tree.d->addDevice(newPartition);
            
            if (newPartition->partitionType() == ExtendedPartition) {
                FreeSpace* logicalSpace = new FreeSpace(newPartition->offset() + SPACE_BETWEEN_LOGICALS, newPartition->size() - SPACE_BETWEEN_LOGICALS, newPartition->name());
                tree.d->addDevice(logicalSpace);
            }
            
            ownerDisk = disk;
            break;
        }
        
        case Action::RemovePartition: {
            Actions::RemovePartitionAction* rpa = dynamic_cast< Actions::RemovePartitionAction* >(action);
            
            QPair< VolumeTree, Partition* > pair = volumeTreeMap.searchTreeWithPartition( rpa->partition() );
            VolumeTree tree = pair.first;
            Partition* partition = pair.second;
            
            /* Don't remove an extended if there is at least one logical depending on it */
            if (partition->partitionType() == ExtendedPartition && !tree.logicalPartitions().isEmpty()) {
                error.setType(PartitioningError::RemovingExtendedError);
                return false;
            }
            
            /* Deletes the partition merging adjacent free blocks if present. */
            tree.d->mergeAndDelete( rpa->partition() );
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
            if (rpa->newSize() < disk->minimumPartitionSize()) {
                error.setType(PartitioningError::PartitionTooSmallError);
                return false;
            }
            
            if (rpa->safe() && (rpa->newSize() < toResize->minimumSize() || rpa->newOffset() != oldOffset)) {
                error.setType(PartitioningError::SafeResizingError);
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

            QStringList supportedFlags = PartitionTableUtils::instance()->supportedFlags( p->partitionTableScheme() );

            /* Check if the action sets supported flags */
            foreach (const QString& flag, mpa->flags()) {
                if (!supportedFlags.contains(flag)) {
                    error.setType(PartitioningError::PartitionFlagsError);
                    error.arg(flag);
                    return false;
                }
            }
            
            if (mpa->isLabelChanged() && !labelChecks(p->partitionTableScheme(), mpa->label())) {
                return false;
            }
            
            if (!mpa->flags().isEmpty() && p->partitionType() == ExtendedPartition) {
                error.setType(PartitioningError::PartitionFlagsError);
                error.arg( mpa->flags().first() );
                return false;
            }
                        
            if (mpa->isLabelChanged()) {
                p->setLabel( mpa->label() );
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
    
    if (!undoOrRedo) {
        actionstack.push(action);
    }
    
    return true;
}

bool VolumeManager::Private::partitionChecks(Action* action)
{
    if (action->isPartitionAction()) {
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

bool VolumeManager::Private::filesystemChecks(const Filesystem& fs)
{        
    FilesystemUtils* fsUtils = FilesystemUtils::instance();
    QStringList supportedFilesystems = fsUtils->supportedFilesystems();
    
    /* The specific filesystem isn't supported, or cannot be created on a partition. */
    if (fs.name() != "unformatted" && !fs.name().isEmpty() &&
        (!supportedFilesystems.contains( fs.name() ) || !fsUtils->filesystemProperty(fs.name(), "can_create").toBool())) {
        error.setType(PartitioningError::FilesystemError);
        error.arg( fs.name() );
        return false;
    }
    
    /* A label was set when not supported, or its length exceeds the maximum allowed */
    if ((!fs.label().isEmpty() && !fsUtils->supportsLabel( fs.name() )) ||
        (fs.label().size() > fsUtils->filesystemProperty(fs.name(), "max_label_len").toInt())) {
        error.setType(PartitioningError::FilesystemLabelError);
        error.arg( fs.name() );
        return false;
    }
    
    /* Ownership was set, but this filesystem doesn't support it. */
    if ((fs.ownerGid() != -1 || fs.ownerUid() != -1) && !fsUtils->filesystemProperty(fs.name(), "supports_unix_owners").toBool()) {
        error.setType(PartitioningError::FilesystemFlagsError);
        error.arg( "owners" );
        return false;
    }

    /* At least one flag of the specified filesystem doesn't actually exist */
    if (!fs.unsupportedFlags().isEmpty()) {
        error.setType(PartitioningError::FilesystemFlagsError);
        error.arg( fs.unsupportedFlags().join(", ") );
        return false;
    }
    
    return true;
}

bool VolumeManager::Private::labelChecks(const QString& scheme, const QString& label)
{
    /* Labels aren't supported in MBR */
    if (scheme == "mbr" && !label.isEmpty()) {
        error.setType(PartitioningError::MBRLabelError);
        return false;
    }
    
    /* In GPT, label must be below 36 characters in size */
    if (scheme == "gpt" && label.size() > 36) {
        error.setType(PartitioningError::LabelTooBigError);
        return false;
    }
    
    return true;
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
    DeviceModified* rightDevice = disk.rightDevice(toResize);
    DeviceModified* leftDevice = disk.leftDevice(toResize);
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
        movePartition(toResize, newOffset, parent, disk);
        resizePartition(toResize, newSize, disk);
    }
    /* If the partition is moved forward beyond the legal boundary... */
    else if (newOffset != oldOffset && (newOffset + oldSize >= rightBoundary)) {
        qlonglong difference = newOffset + oldSize - rightBoundary;
        
        /* ... but the size is reduced of at least that quantity, OK */
        if ((oldSize - newSize) < difference) {
            error.setType(PartitioningError::ResizeOutOfBoundsError);
            return false;
        }
        
        resizePartition(toResize, newSize, disk);
        movePartition(toResize, newOffset, parent, disk);
    }
    else { /* no specific order needed */
        resizePartition(toResize, newSize, disk);
        movePartition(toResize, newOffset, parent, disk);
    }
    
    return true;
}

void VolumeManager::Private::resizePartition(Partition* partition,
                                             qulonglong newSize,
                                             VolumeTree& disk)
{  
    if (newSize == partition->size()) {
        return;
    }
    
    DeviceModified* rightDevice = disk.rightDevice(partition);
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
    
    /* Creates a new object for the free space left on the right */
    if (!rightDevice || rightDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qlonglong spaceOffset = partition->offset() + newSize;
        qlonglong spaceSize = (rightDevice->offset() - (partition->offset() + newSize) - spaceBetween);
        
//         qDebug() << "RESIZE: new space on the right of offset" << spaceOffset << "and" << spaceSize;
        
        if (spaceSize > 0) {
            FreeSpace* freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
            disk.d->addDevice(freeSpaceRight);
        }
    } else {
        FreeSpace* spaceRight = dynamic_cast< FreeSpace* >(rightDevice);
        qlonglong rightOffset = partition->offset() + newSize;
        qlonglong rightSize = spaceRight->size() - (newSize - partition->size());
        
//         qDebug() << "RESIZE: space on the right has new offset" << rightOffset << "and new size" << rightSize;
                        
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
                                           VolumeTree& tree)
{
    if (newOffset == partition->offset()) {
        return;
    }
    
    DeviceModified* leftDevice = tree.leftDevice(partition);
    DeviceModified* rightDevice = tree.rightDevice(partition);
    qulonglong oldOffset = partition->offset();
    FreeSpace *freeSpaceRight = 0;
    FreeSpace *freeSpaceLeft = 0;
    qulonglong spaceBetween = (partition->partitionType() == LogicalPartition) ? SPACE_BETWEEN_LOGICALS : 0;
    
    /* Creates a new object for the space left on the left */
    if (!leftDevice || leftDevice->deviceType() != DeviceModified::FreeSpaceDevice) {
        qulonglong spaceOffset = (leftDevice) ? (leftDevice->rightBoundary()) : (parent->offset() + spaceBetween);
        qulonglong spaceSize = newOffset - spaceOffset - spaceBetween;
        
//         qDebug() << "MOVE: new space on the left of offset" << spaceOffset << "and" << spaceSize;
        
        if (spaceSize > 0) {
            freeSpaceLeft = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
        }
    } else { /* there's some free space immediately before: changes its size accordingly */
        qulonglong leftSize = leftDevice->size() - (oldOffset - newOffset);

//         qDebug() << "MOVE: space on the left has now size" << leftSize;
        
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
                        
//         qDebug() << "MOVE: new space on the right of offset" << spaceOffset << "and" << spaceSize;
        
        if (spaceSize > 0) {
            freeSpaceRight = new FreeSpace(spaceOffset, spaceSize, partition->parentName());
            tree.d->addDevice(freeSpaceRight);
        }
    } else {
        qulonglong rightOffset = newOffset + partition->size();
        qulonglong rightSize = rightDevice->size() - (newOffset - oldOffset);
                
//         qDebug() << "RESIZE: space on the right has now offset" << rightOffset << "and" << rightSize;
        
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
        tree.d->addDevice(freeSpaceLeft);
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
     * If all partition table signatures have been eliminated, don't do it
     * as we aren't able to create anything on the disk.
     */
    if (!scheme.isEmpty()) {
        FreeSpace* bigFreeSpace = new FreeSpace(disk->offset(), disk->size(), disk->name());
        diskNode->addChild(bigFreeSpace);
    }

    return true;
}

}
}
