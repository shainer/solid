#include "freespace.h"

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
    
    ~Private()
    {}
    
    qulonglong offset;
    qulonglong size;
};
    
FreeSpace::FreeSpace(qulonglong offset, qulonglong size, const QString& parentUdi)
    : DeviceModified()
    , d( new Private( offset, size ) )
{
    /* FIXME: ci resta il /dev davanti, attenzione */
    QString uniqueName = "Free space of offset " + QString::number(d->offset) + " and size " + QString::number(d->size);
    DeviceModified::setName(uniqueName);
    DeviceModified::setParentName(parentUdi);
}

FreeSpace::~FreeSpace()
{
    delete d;
}

DeviceModified::DeviceModifiedType FreeSpace::deviceType() const
{
    return FreeSpaceDevice;
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