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

QString DeviceModified::udiToName(const QString& udi)
{
    QString tmp = udi.split("/").last();
    tmp.prepend("/dev/");
    return tmp;
}

DeviceModified::DeviceModified(DeviceInterface* iface)
    : d( new Private(iface) )
{}

DeviceModified::DeviceModified()
    : d( new Private )
{}

DeviceModified::~DeviceModified()
{
    delete d;
}

QString DeviceModified::name() const
{
    return d->name;
}

QString DeviceModified::parentName() const
{
    return d->parentName;
}

bool DeviceModified::existent() const
{
    return d->existent;
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