#include "storagedrivemodified.h"

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

class StorageDriveModifiedPrivate : public QSharedData
{
public:
    StorageDriveModifiedPrivate(StorageDrive *i)
        : QSharedData()
        , iface(i)
        , size(i->size())
    {}
    
    StorageDrive* iface;
    qulonglong size;
};

StorageDriveModified::StorageDriveModified(StorageDrive* drive)
    : DeviceModified(drive)
    , d(new StorageDriveModifiedPrivate( drive ))
{}

StorageDriveModified::StorageDriveModified(const StorageDriveModified& other)
    : DeviceModified(other.d->iface)
    , d(other.d)
{}

StorageDriveModified::~StorageDriveModified()
{}

DeviceModified::DeviceModifiedType StorageDriveModified::deviceType() const
{
    return StorageDriveType;
}

qulonglong StorageDriveModified::size() const
{
    return d->size;
}

void StorageDriveModified::setSize(qulonglong s)
{
    d->size = s;
}
    
}
}
}