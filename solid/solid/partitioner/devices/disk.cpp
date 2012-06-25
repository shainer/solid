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
#include <solid/partitioner/devices/disk.h>
#include <solid/partitioner/utils/utils.h>
#include <QtCore/QDebug>

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
        , scheme(i->partitionTableScheme())
    {

    }
    
    Private(qulonglong s, const QString& t)
        : size(s)
        , scheme(t)
    {}
    
    StorageDrive* iface;
    qulonglong size;
    QString scheme;
};

Disk::Disk(StorageDrive* drive)
    : DeviceModified(drive)
    , d( new Private( drive ) )
{}

Disk::Disk(Disk* other)
    : d( new Private(other->size(), other->partitionTableScheme()) )
{
    DeviceModified::setName(other->name());
    DeviceModified::setParentName(other->parentName());
}

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

    if (d->scheme == "gpt") {
        s -= 512 * 34; /* secondary GPT table which replicates the first at the end for safety purposes */
    }

    return s;
}

qulonglong Disk::offset() const
{
    qulonglong off = 0;

    if (d->scheme == "mbr") {
        off = 1024 * 1024; /* the first MB is reserved */
    }
    else if (d->scheme == "gpt") {
        /* FIXME: this value assumes the LBA size is the standard 512 bytes. However, this is not always the case. */
        off = 512 * 40;
   }

    return off;
}

QString Disk::partitionTableScheme() const
{
    return d->scheme;
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
    Q_UNUSED(s)
}

void Disk::setPartitionTableScheme(const QString& scheme)
{
    d->scheme = scheme;
}

qulonglong Disk::minimumPartitionSize() const
{
    qulonglong megabyte = 1024 * 1024;
    qulonglong onePercent = d->size / 100;
    qulonglong min = (onePercent > megabyte) ? megabyte : onePercent;
    
    return min;
}

}
}
}
