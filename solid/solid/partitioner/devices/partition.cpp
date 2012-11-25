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
#include <solid/partitioner/devices/partition.h>
#include <solid/partitioner/utils/utils.h>
#include <solid/partitioner/utils/filesystemutils.h>
#include <solid/partitioner/resizehelper.h>

#include <QtCore/QEventLoop>

using namespace KAuth;

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

UpdaterLoop::UpdaterLoop(Solid::Partitioner::Devices::Partition* const parent)
    : QEventLoop()
    , parent( parent )
{}

void UpdaterLoop::quit(ActionReply reply)
{
    /*
     * Takes the return value from KAuth's invocation and sets the minimum size of
     * the partition.
     */
    if (!reply.succeeded()) {
        parent->setMinimumSize( parent->size() ); /* there was some error, the user cannot resize */
        QEventLoop::quit();
        return;
    }

    if (reply.data().contains("minimumPartitionSize")) {
        qlonglong minSize( reply.data().value("minimumPartitionSize").toLongLong() );
        parent->setMinimumSize( (minSize != -1) ? minSize : parent->size() );
    }

    QEventLoop::quit();
}
    
using namespace Utils;
using namespace Backends::UDisks;
    
class Partition::Private
{
public:
    Private(StorageVolume* v)
        : access(0)
        , ignored(v->isIgnored())
        , isFsExistent(true)
        , usage(v->usage())
        , label(v->label())
        , uuid(v->uuid())
        , size(v->size())
        , minimumSize(-1)
        , offset(v->offset())
        , partitionType(PrimaryPartition)
        , partitionTypeString(v->partitionType())
        , scheme(v->partitionTableScheme())
        , flags(v->flags())
    {
        QString fsName = FilesystemUtils::instance()->filesystemNameFromId( v->fsType() );
        filesystem = Filesystem(fsName);
        
        /* For an existing partition we know the type string but not the actual type (primary, extended, ...) */
        setTypeFromString();
    }
    
    Private(Actions::CreatePartitionAction* action, const QString& s)
        : access(0)
        , ignored(false)
        , isFsExistent(false)
        , usage(StorageVolume::FileSystem)
        , filesystem( action->filesystem() )
        , label(action->label())
        , size(action->size())
        , minimumSize(-1)
        , offset(action->offset())
        , partitionType(action->partitionType())
        , scheme(s)
        , flags(action->flags())
    {
        /*
         * For a partition created by the application, we know the filesystem and if it's extended, primary, etc.,
         * but with this method we determine the type string according to the partition table scheme. This function
         * is called also when either the partitionType or the filesystem property changes.
         */
        setStringFromType();
    }
    
    Private()
    {}
    
    void setTypeFromString()
    {
        if (partitionTypeString == EXTENDED_TYPE_STRING || partitionTypeString == EXTENDED_TYPE_STRING_LBA) {
            partitionType = ExtendedPartition;
        }
    }
    
    void setStringFromType()
    {
        QString type = (partitionType == ExtendedPartition) ? "extended" : Utils::FilesystemUtils::instance()->filesystemIdFromName(filesystem.name());
        partitionTypeString = Utils::PartitionTableUtils::instance()->typeString(scheme, type);
    }
    
    UDisksStorageAccess* access;
    
    bool ignored;
    bool isFsExistent;
    StorageVolume::UsageType usage;
    Filesystem filesystem;
    QString label;
    QString uuid;
    qulonglong size;
    qlonglong minimumSize;
    qulonglong offset;
    PartitionType partitionType;
    QString partitionTypeString;
    QString scheme;
    QStringList flags;
};
    
Partition::Partition(Device& dev)
    : d( new Private( dev.as<StorageVolume>() ) )
{
    if (dev.is<StorageAccess>()) {
        UDisksDevice* udisksDevice = new UDisksDevice( dev.udi() );
        setAccess( new UDisksStorageAccess(udisksDevice) );
    }

    DeviceModified::setName( dev.udi() );
    DeviceModified::setDescription( dev.udi() );
    DeviceModified::setParentName( dev.parentUdi() );
}

Partition::Partition(Actions::CreatePartitionAction* action, const QString& scheme)
    : DeviceModified()
    , d( new Private(action, scheme) )
{    
    DeviceModified::setName( action->partitionName() );
    DeviceModified::setDescription( action->partitionName() );
    DeviceModified::setParentName( action->disk() );
}

Partition::Partition()
    : d( new Private )
{}

Partition::~Partition()
{
    delete d;
}

DeviceModified* Partition::copy() const
{
    Partition* partitionCopy = new Partition;
    
    partitionCopy->setAccess( access() );
    partitionCopy->setFilesystem( filesystem() );
    partitionCopy->setFlags( flags() );
    partitionCopy->setIgnored( ignored() );
    partitionCopy->setLabel( label() );
    partitionCopy->setOffset( offset() );
    partitionCopy->setPartitionTableScheme( partitionTableScheme() );
    partitionCopy->setPartitionTypeString( partitionTypeString() );
    partitionCopy->setPartitionType( partitionType() );
    partitionCopy->setSize( size() );
    partitionCopy->setUsage( usage() );
    partitionCopy->setDescription( description() );
    partitionCopy->setName( name() );
    partitionCopy->setParentName( parentName() );
    partitionCopy->d->isFsExistent = d->isFsExistent;
    partitionCopy->setMinimumSize( d->minimumSize );
    
    return partitionCopy;
}

DeviceModified::DeviceModifiedType Partition::deviceType() const
{
    return DeviceModified::PartitionDevice;
}

QString Partition::uuid() const
{
    return d->uuid;
}

bool Partition::ignored() const
{
    return d->ignored;
}

StorageVolume::UsageType Partition::usage() const
{
    return d->usage;
}
    
Filesystem Partition::filesystem() const
{
    return d->filesystem;
}
    
QString Partition::label() const
{
    return d->label;
}

PartitionType Partition::partitionType() const
{
    return d->partitionType;
}

QString Partition::partitionTypeString() const
{
    return d->partitionTypeString;
}

QString Partition::partitionTableScheme() const
{
    return d->scheme;
}

QStringList Partition::flags() const
{
    return d->flags;
}

qulonglong Partition::size() const
{
    return d->size;
}

qulonglong Partition::minimumSize()
{
    if (d->minimumSize == -1) {
        computeMinimumSize();
    }
    
    return (qulonglong)d->minimumSize;
}

qulonglong Partition::offset() const
{
    return d->offset;
}

qulonglong Partition::rightBoundary() const
{
    return (d->offset + d->size);
}

bool Partition::isMounted() const
{
    if (!d->access) {
        return false;
    }
    
    return d->access->isAccessible();
}

QString Partition::mountFile() const
{
    if (!d->access) {
        return QString();
    }

    return d->access->filePath();
}

UDisksStorageAccess* Partition::access() const
{
    return d->access;
}

void Partition::setIgnored(bool ign)
{
    d->ignored = ign;
}

void Partition::setPartitionType(PartitionType type)
{
    d->partitionType = type;
    d->setStringFromType();
}

void Partition::setPartitionTypeString(const QString& type)
{
    d->partitionTypeString = type;
    d->setTypeFromString();
}

void Partition::setPartitionTableScheme(const QString& scheme)
{
    d->scheme = scheme;
}

void Partition::setUsage(StorageVolume::UsageType usage)
{
    d->usage = usage;
}

void Partition::setFilesystem(const Filesystem& fs)
{
    d->filesystem = fs;
    d->setStringFromType();
    
    d->isFsExistent = false;
    computeMinimumSize();
}

void Partition::setLabel(const QString& label)
{
    d->label = label;
}

void Partition::setSize(qulonglong size)
{
    d->size = size;
}

void Partition::setOffset(qulonglong offset)
{
    d->offset = offset;
}

void Partition::setFlags(const QStringList& flags)
{
    d->flags = flags;
}

void Partition::setAccess(UDisksStorageAccess* access)
{
    d->access = access;
}

void Partition::setMinimumSize(qulonglong minSize)
{
    d->minimumSize = minSize;
}

void Partition::computeMinimumSize()
{
    QString fsName = d->filesystem.name();
    QStringList notSupported = QStringList() << "NILFS2" << "BTRFS" << "Minix";

    /* Note that for swap space we don't need to preserve data, so a simple value of 0 is returned */
    if (fsName.isEmpty() || fsName == "unformatted" || fsName == "Swap Space") {
        d->minimumSize = 0;
        return;
    }
    
    if (notSupported.contains(fsName)) {
        d->minimumSize = d->size;
        return;
    }
    
    if (!d->isFsExistent) {
        d->minimumSize = FilesystemUtils::instance()->minimumFilesystemSize(fsName);
        return;
    }
    
    qRegisterMetaType<ActionReply>("ActionReply");
    KAuth::Action asyncAction("org.solid.partitioner.resize.resizehelper");
    
    asyncAction.addArgument("minSize", true);
    asyncAction.addArgument("partition", name());
    asyncAction.addArgument("disk", parentName());
    asyncAction.addArgument("filesystem", d->filesystem.name());
    asyncAction.addArgument("path", QString::fromAscii( getenv("PATH") ));
    
    asyncAction.setExecutesAsync(true); /* it must be asynchronous to avoid timeouts */
    UpdaterLoop loop( this );
        
    connect(asyncAction.watcher(), SIGNAL(actionPerformed(ActionReply)), &loop, SLOT(quit(ActionReply)));
    ActionReply earlyReply( asyncAction.execute("org.solid.partitioner.resize") );
    
    if (earlyReply.failed()) {
        return;
    }

    loop.exec(); /* wait until the action has terminated */
}

}
}
}

#include "partition.moc"