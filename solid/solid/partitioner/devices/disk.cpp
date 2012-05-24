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
    {}
    
    StorageDrive* iface;
    qulonglong size;
};

Disk::Disk(StorageDrive* drive)
    : DeviceModified(drive)
    , d( new Private( drive ) )
{}

Disk::~Disk()
{}

DeviceModified::DeviceModifiedType Disk::deviceType() const
{
    return DiskDevice;
}

qulonglong Disk::size() const
{
    return d->size;
}

void Disk::setSize(qulonglong s)
{
    d->size = s;
}
    
}
}
}