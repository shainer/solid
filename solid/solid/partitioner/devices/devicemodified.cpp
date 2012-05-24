#include "devicemodified.h"
#include <QtCore/QStringList>
#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

class DeviceModified::Private
{
public:
    Private(DeviceInterface* i)
        : iface(i)
        , existent(i->isValid())
    {}
    
    Private()
        : iface(0)
        , existent(true)
    {}
    
    DeviceInterface* iface;
    QString name;
    QString parentName;
    bool existent;
};
    
DeviceModified::DeviceModified(DeviceInterface* iface)
    : d( new Private(iface) )
{}

DeviceModified::DeviceModified()
    : d( new Private )
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

bool DeviceModified::operator==(const DeviceModified& other) const
{
    return d->name == other.name();
}
    
}
}
}