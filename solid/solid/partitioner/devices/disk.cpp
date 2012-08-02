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
        : size( i->size() )
        , sectorSize(512) /* default value, will be changed when we know the disk's name */
        , scheme( i->partitionTableScheme() )
    {}
    
    Private()
        : sectorSize(512)
    {}
    
    qulonglong size;
    qulonglong sectorSize;
    QString scheme;
};

Disk::Disk(Device dev)
    : DeviceModified( dev.as<StorageDrive>() )
    , d( new Private( dev.as<StorageDrive>() ) )
{
    setName( dev.udi() );
}

Disk::Disk()
    : d( new Private )
{}

Disk::~Disk()
{
    delete d;
}

DeviceModified* Disk::copy() const
{
    Disk* copyDisk = new Disk;
    copyDisk->setOffset( offset() );
    copyDisk->setPartitionTableScheme( partitionTableScheme() );
    copyDisk->setSize( size() );
    copyDisk->setDescription( description() );
    copyDisk->setName( name() );
    copyDisk->setParentName( parentName() );
    
    return copyDisk;
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
        s -= d->sectorSize * 34; /* secondary GPT table which replicates the first at the end for safety purposes */
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
        off = d->sectorSize * 40;
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

void Disk::setName(const QString& name)
{
    DeviceModified::setName(name);
    d->sectorSize = Utils::sectorSize(name);
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
