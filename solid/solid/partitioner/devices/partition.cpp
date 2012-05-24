#include <solid/partitioner/devices/partition.h>
#include <backends/udisks/udisksdevice.h>
#include <QtCore/QDebug>
#include <kio/global.h>

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

class Partition::Private
{
public:
    Private(StorageVolume* v)
        : iface(v)
        , ignored(v->isIgnored())
        , usage(v->usage())
        , filesystem(v->fsType())
        , label(v->label())
        , uuid(v->uuid())
        , size(v->size())
        , offset(v->offset())
    {}
    
    Private(Actions::CreatePartitionAction *action)
        : iface(0)
        , ignored(false)
        , usage(StorageVolume::FileSystem)
        , size(action->size())
        , offset(action->offset())
        , partitionType(action->partitionType())
    {}
    
    StorageVolume *iface;
    
    bool ignored;
    StorageVolume::UsageType usage;
    QString filesystem;
    QString label;
    QString uuid;
    qulonglong size;
    qulonglong offset;
    PartitionType partitionType;
};
    
Partition::Partition(StorageVolume* volume)
    : DeviceModified(volume)
    , d( new Private(volume) )
{}

Partition::Partition(Actions::CreatePartitionAction* action)
    : DeviceModified()
    , d( new Private(action) )
{
    DeviceModified::setParentName(action->disk());
}

Partition::~Partition()
{}

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
    
QString Partition::filesystem() const
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

void Partition::setIgnored(bool ign)
{
    d->ignored = ign;
}

void Partition::setPartitionType(PartitionType type)
{
    d->partitionType = type;
}

void Partition::setUsage(StorageVolume::UsageType usage)
{
    d->usage = usage;
}

void Partition::setFilesystem(const QString& fs)
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

}
}
}