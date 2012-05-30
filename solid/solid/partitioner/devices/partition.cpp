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

using namespace Utils;

class Partition::Private
{
public:
    Private(StorageVolume* v)
        : iface(v)
        , ignored(v->isIgnored())
        , usage(v->usage())
        , filesystem( Utils::Filesystem(v->fsType()) )
        , label(v->label())
        , uuid(v->uuid())
        , size(v->size())
        , offset(v->offset())
        , flags(v->flags())
    {}
    
    Private(Actions::CreatePartitionAction *action)
        : iface(0)
        , ignored(false)
        , usage(StorageVolume::FileSystem)
        , label(action->label())
        , size(action->size())
        , offset(action->offset())
        , partitionType(action->partitionType())
        , flags(action->flags())
    {}
    
    StorageVolume *iface;
    
    bool ignored;
    StorageVolume::UsageType usage;
    Utils::Filesystem filesystem;
    QString label;
    QString uuid;
    qulonglong size;
    qulonglong offset;
    PartitionType partitionType;
    QStringList flags;
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
    DeviceModified::setName("New partition");
}

Partition::~Partition()
{
    delete d;
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

bool Partition::isFlagSet(const QString& flag) const
{
    return d->flags.contains(flag);
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

void Partition::setFlag(const QString& flag)
{
    d->flags.append(flag);
}

void Partition::setFlags(const QStringList& flags)
{
    d->flags = flags;
}

void Partition::unsetFlag(const QString& flag)
{
    d->flags.removeOne(flag);
}

}
}
}
