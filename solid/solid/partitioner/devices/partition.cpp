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
#include <backends/udisks/udisksdevice.h>
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
    Private(StorageVolume* v, StorageAccess* a)
        : access(a)
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
    {
        QString fsName = FilesystemUtils::instance()->filesystemNameFromId( v->fsType() );
        filesystem = Utils::Filesystem(fsName);
        
        setTypeFromString();
    }
    
    Private(Actions::CreatePartitionAction* action)
        : access(0)
        , ignored(false)
        , usage(StorageVolume::FileSystem)
        , label(action->label())
        , size(action->size())
        , offset(action->offset())
        , partitionType(action->partitionType())
        , flags(action->flags())
    {}
    
    Private()
    {}
    
    void setTypeFromString()
    {
        if (partitionTypeString == EXTENDED_TYPE_STRING || partitionTypeString == EXTENDED_TYPE_STRING_LBA) {
            partitionType = ExtendedPartition;
        }
    }
    
    StorageAccess* access;
    
    bool ignored;
    StorageVolume::UsageType usage;
    Utils::Filesystem filesystem;
    QString label;
    QString uuid;
    qulonglong size;
    qulonglong offset;
    PartitionType partitionType;
    QString partitionTypeString;
    QString scheme;
    QStringList flags;
};
    
Partition::Partition(Device& dev)
{
    StorageVolume* volume = dev.as<StorageVolume>();
    StorageAccess* access = 0;
    
    if (dev.is<StorageAccess>()) {
        access = dev.as<StorageAccess>();
    }
    
    d = new Private(volume, access);
}

Partition::Partition(Actions::CreatePartitionAction* action)
    : DeviceModified()
    , d( new Private(action) )
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
    partitionCopy->d->access = access();
    
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
    
Utils::Filesystem Partition::filesystem() const
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

void Partition::setFilesystem(const Utils::Filesystem& fs)
{
    d->filesystem = fs;
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

}
}
}
