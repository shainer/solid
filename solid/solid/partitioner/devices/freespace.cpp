#include "freespace.h"

namespace Solid
{
namespace Partitioner
{
namespace Devices
{

FreeSpace::FreeSpace(qulonglong offset, qulonglong size, const QString& parentUdi)
    : DeviceModified()
    , d( new FreeSpacePrivate( offset, size ) )
{
    DeviceModified::setParentName(parentUdi);
}

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