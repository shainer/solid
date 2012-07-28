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

#include <solid/storageaccess.h>

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

using namespace Utils;

class Partition::Private
{
public:
    Private(StorageVolume* v)
        : access(0)
        , ignored(v->isIgnored())
        , usage(v->usage())
        , label(v->label())
        , uuid(v->uuid())
        , size(v->size())
        , offset(v->offset())
        , partitionType(PrimaryPartition)
        , partitionTypeString(v->partitionType())
        , scheme(v->partitionTableScheme())
        , flags(v->flags())
        , mounted(false)
    {
        QString fsName = FilesystemUtils::instance()->filesystemNameFromId( v->fsType() );
        filesystem = Filesystem(fsName);
        
        /* For an existing partition we know the type string but not the actual type (primary, extended, ...) */
        setTypeFromString();
    }
    
    Private(Actions::CreatePartitionAction* action, const QString& s)
        : access(0)
        , ignored(false)
        , usage(StorageVolume::FileSystem)
        , filesystem( action->filesystem() )
        , label(action->label())
        , size(action->size())
        , offset(action->offset())
        , partitionType(action->partitionType())
        , scheme(s)
        , flags(action->flags())
        , mounted(false)
        , mountFile( QString() )
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
    
    StorageAccess* access;
    
    bool ignored;
    StorageVolume::UsageType usage;
    Filesystem filesystem;
    QString label;
    QString uuid;
    qulonglong size;
    qulonglong offset;
    PartitionType partitionType;
    QString partitionTypeString;
    QString scheme;
    QStringList flags;
    
    bool mounted;
    QString mountFile;
};
    
Partition::Partition(Device& dev)
    : d( new Private( dev.as<StorageVolume>() ) )
{
    if (dev.is<StorageAccess>()) {
        setAccess( dev.as<StorageAccess>() );
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
    DeviceModified::setParentName(action->disk());
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
    partitionCopy->setAccess( access() );
    
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
    return d->mounted;
}

QString Partition::mountFile() const
{
    return d->mountFile;
}

StorageAccess* Partition::access() const
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

void Partition::setAccess(StorageAccess* access)
{
    if (access) {
        d->access = access;
        d->mounted = access->isAccessible();
        d->mountFile = access->filePath();
        
        QObject::connect(access,
                            SIGNAL(accessibilityChanged(bool, const QString &)),
                            this,
                            SLOT(doAccessibilityChanged(bool, const QString &)));
    }
}

void Partition::doAccessibilityChanged(bool accessible, const QString& udi)
{
    Q_UNUSED(udi)
    d->mounted = accessible;
    d->mountFile = d->access->filePath();
}

}
}
}

#include "partition.moc"