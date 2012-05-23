#include "devicemodified.h"

namespace Solid
{
namespace Partitioner
{
namespace Devices
{
    
DeviceModified::DeviceModified(DeviceInterface* iface)
    : d(new DeviceModifiedPrivate(iface))
{}

DeviceModified::DeviceModified()
    : d(new DeviceModifiedPrivate)
{}

DeviceModified::DeviceModified(const DeviceModified& other)
    : d(other.d)
{}

DeviceModified::~DeviceModified()
{}

QString DeviceModified::udi() const
{
    return d->udi;
}

QString DeviceModified::parentUdi() const
{
    return d->parentUdi;
}

bool DeviceModified::existent() const
{
    return d->existent;
}

void DeviceModified::setExistent(bool ex)
{
    d->existent = ex;
}

void DeviceModified::setUdi(const QString& udi)
{
    d->udi = udi;
}

void DeviceModified::setParentUdi(const QString& udi)
{
    d->parentUdi = udi;
}
    
}
}
}