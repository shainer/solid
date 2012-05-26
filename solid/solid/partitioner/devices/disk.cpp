#include <solid/partitioner/devices/disk.h>

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

class Disk::Private
{
public:
    Private(StorageDrive *i)
        : iface(i)
        , size(i->size())
    {
        if (i->partitionTableScheme() == "mbr") {
            ptableType = Utils::MBR;
        }
        else if (i->partitionTableScheme() == "gpt") {
            ptableType = Utils::GPT;
        }
        else if (i->partitionTableScheme() == "apm") {
            ptableType = Utils::APM;
        }
        else {
            ptableType = Utils::None;
        }
    }
    
    StorageDrive* iface;
    qulonglong size;
    Utils::PTableType ptableType;
};

Disk::Disk(StorageDrive* drive)
    : DeviceModified(drive)
    , d( new Private( drive ) )
{}

Disk::~Disk()
{
    delete d;
}

DeviceModified::DeviceModifiedType Disk::deviceType() const
{
    return DiskDevice;
}

qulonglong Disk::size() const
{
    qulonglong s = d->size;
    s -= offset();

    if (d->ptableType == Utils::GPT) {
        s -= (512 * 8 + 16 * 1024 * 8);
    }

    return s;
}

qulonglong Disk::offset() const
{
    qulonglong off = 0;

    switch (d->ptableType)
    {
        case Utils::MBR: {
            off += 512 * 8;
            break;
        }

        case Utils::GPT: {
            off += (512 * 8 + 16 * 1024 * 8);
            off += 512 * 8;
            break;
        }

        default:
            break;
    }

    return off;
}

Utils::PTableType Disk::partitionTableScheme() const
{
    return d->ptableType;
}

qulonglong Disk::rightBoundary() const
{
    return d->size;
}

void Disk::setOffset(qulonglong offset)
{
    Q_UNUSED(offset)
}

void Disk::setSize(qulonglong s)
{
    d->size = s;
}

void Disk::setPartitionTableScheme(Utils::PTableType type)
{
    d->ptableType = type;
}
    
}
}
}
