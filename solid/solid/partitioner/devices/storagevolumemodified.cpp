#include "storagevolumemodified.h"

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

class StorageVolumeModifiedPrivate : public QSharedData
{
public:
    StorageVolumeModifiedPrivate(StorageVolume* v)
        : iface(v)
        , ignored(v->isIgnored())
        , usage(v->usage())
        , filesystem(v->fsType())
        , label(v->label())
        , uuid(v->uuid())
        , size(v->size())
        , offset(v->offset())
    {}
    
    StorageVolumeModifiedPrivate(const StorageVolumeModifiedPrivate& other)
        : QSharedData(other)
        , iface(other.iface)
        , ignored(other.ignored)
        , usage(other.usage)
        , filesystem(other.filesystem)
        , label(other.label)
        , uuid(other.uuid)
        , size(other.size)
        , offset(other.offset)
    {}
    
    StorageVolume *iface;
    
    bool ignored;
    StorageVolume::UsageType usage;
    QString filesystem;
    QString label;
    QString uuid;
    qulonglong size;
    qulonglong offset;
    StorageVolumeModified::PartitionType partitionType;
};
    
StorageVolumeModified::StorageVolumeModified(StorageVolume* volume)
    : DeviceModified(volume)
    , d(new StorageVolumeModifiedPrivate(volume))
{}

StorageVolumeModified::StorageVolumeModified(const StorageVolumeModified& other)
    : DeviceModified(other.d->iface)
    , d(other.d)
{}

StorageVolumeModified::~StorageVolumeModified()
{}

DeviceModified::DeviceModifiedType StorageVolumeModified::deviceType() const
{
    return DeviceModified::StorageVolumeType;
}

QString StorageVolumeModified::uuid() const
{
    return d->uuid;
}

bool StorageVolumeModified::ignored() const
{
    return d->ignored;
}

StorageVolume::UsageType StorageVolumeModified::usage() const
{
    return d->usage;
}
    
QString StorageVolumeModified::filesystem() const
{
    return d->filesystem;
}
    
QString StorageVolumeModified::label() const
{
    return d->label;
}

StorageVolumeModified::PartitionType StorageVolumeModified::partitionType() const
{
    return d->partitionType;
}

qulonglong StorageVolumeModified::size() const
{
    return d->size;
}

qulonglong StorageVolumeModified::offset() const
{
    return d->offset;
}

qulonglong StorageVolumeModified::rightBoundary() const
{
    return (d->offset + d->size);
}

void StorageVolumeModified::setIgnored(bool ign)
{
    d->ignored = ign;
}

void StorageVolumeModified::setPartitionType(StorageVolumeModified::PartitionType type)
{
    d->partitionType = type;
}

void StorageVolumeModified::setUsage(StorageVolume::UsageType usage)
{
    d->usage = usage;
}

void StorageVolumeModified::setFilesystem(const QString& fs)
{
    d->filesystem = fs;
}

void StorageVolumeModified::setLabel(const QString& label)
{
    d->label = label;
}

void StorageVolumeModified::setSize(qulonglong size)
{
    d->size = size;
}

void StorageVolumeModified::setOffset(qulonglong offset)
{
    d->offset = offset;
}

}
}
}