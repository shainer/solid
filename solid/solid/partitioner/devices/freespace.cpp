#include "freespace.h"

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

FreeSpace::FreeSpace(qulonglong size, qulonglong offset, const QString& parentUdi)
    : DeviceModified()
    , d( new FreeSpacePrivate( size, offset, parentUdi ) )
{}

FreeSpace::FreeSpace(const FreeSpace& other)
    : DeviceModified()
    , d( other.d )
{}

FreeSpace::~FreeSpace()
{}

DeviceModified::DeviceModifiedType FreeSpace::deviceType() const
{
    return FreeSpaceType;
}

qulonglong FreeSpace::size() const
{
    return d->size;
}

qulonglong FreeSpace::offset() const
{
    return d->offset;
}

QString FreeSpace::parentUdi() const
{
    return d->parentUdi;
}

void FreeSpace::setSize(qulonglong s)
{
    d->size = s;
}

void FreeSpace::setOffset(qulonglong o)
{
    d->offset = o;
}

}
}
}