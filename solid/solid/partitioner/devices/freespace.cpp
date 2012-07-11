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
#include <solid/partitioner/devices/freespace.h>
#include <solid/partitioner/utils/utils.h>
#include <backends/udisks/udisksdevice.h>

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

class FreeSpace::Private
{
public:
    Private(qulonglong o, qulonglong s)
        : offset(o)
        , size(s)
    {}
    
    Private()
    {}
    
    ~Private()
    {}
    
    void setDeviceName()
    {        
        QString uniqueName = "Free space of offset %0 and size %1";
        
        q->setName( uniqueName.arg(QString::number(offset), QString::number(size)) );
        q->setDescription( "Free space" );
    }
    
    FreeSpace* q;
    qulonglong offset;
    qulonglong size;
};
    
FreeSpace::FreeSpace(qulonglong offset, qulonglong size, const QString& parentUdi)
    : DeviceModified()
    , d( new Private( offset, size ) )
{
    d->q = this;
    d->setDeviceName();
    DeviceModified::setParentName(parentUdi);
}

FreeSpace::FreeSpace()
    : d( new Private )
{
    d->q = this;
}

FreeSpace::~FreeSpace()
{
    delete d;
}

DeviceModified::DeviceModifiedType FreeSpace::deviceType() const
{
    return FreeSpaceDevice;
}

DeviceModified* FreeSpace::copy() const
{
    FreeSpace* spaceCopy = new FreeSpace;
    
    spaceCopy->setOffset( offset() );
    spaceCopy->setSize( size() );
    spaceCopy->setDescription( description() );
    spaceCopy->setName( name() );
    spaceCopy->setParentName( parentName() );
    
    return spaceCopy;
}

qulonglong FreeSpace::size() const
{
    return d->size;
}

qulonglong FreeSpace::offset() const
{
    return d->offset;
}

qulonglong FreeSpace::rightBoundary() const
{
    return (d->offset + d->size);
}

bool FreeSpace::isMinimumSize() const
{
    qulonglong onePercentDiskSize = Utils::getDiskSize(parentName()) / 100;
    qulonglong megabyte = (1024 * 1024);
    qulonglong minimum = (onePercentDiskSize > megabyte) ? megabyte : onePercentDiskSize;
    
    return (d->size >= minimum);
}

void FreeSpace::setSize(qulonglong s)
{
    d->size = s;
    d->setDeviceName();
}

void FreeSpace::setOffset(qulonglong o)
{
    d->offset = o;
    d->setDeviceName();
}

}
}
}