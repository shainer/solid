#include "devicemodified.h"
#include <QtCore/QStringList>
#include <QtCore/QDebug>

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

QString DeviceModified::name() const
{
    return d->name;
}

QString DeviceModified::parentName() const
{
    return d->parentName;
}

qulonglong DeviceModified::offset() const
{
    return 0;
}

bool DeviceModified::existent() const
{
    return d->existent;
}

void DeviceModified::setExistent(bool ex)
{
    d->existent = ex;
}

void DeviceModified::setName(const QString& udi)
{
    QString n = udi.split("/").last();
    n.prepend("/dev/");
    
    d->name = n;
}

void DeviceModified::setParentName(const QString& udi)
{
    QString n = udi.split("/").last();
    n.prepend("/dev/");
    
    d->parentName = n;
}
    
}
}
}