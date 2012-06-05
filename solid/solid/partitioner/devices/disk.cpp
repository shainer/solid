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
    {
        if (i->partitionTableScheme() == "mbr") {
            ptableType = Utils::MBR;
        }
        else if (i->partitionTableScheme() == "gpt") {
            ptableType = Utils::GPT;
        }
        else if (i->partitionTableScheme() == "apm") {
            ptableType = Utils::APM;
        }
        else {
            ptableType = Utils::None;
        }
    }
    
    StorageDrive* iface;
    qulonglong size;
    Utils::PTableType ptableType;
};

Disk::Disk(StorageDrive* drive)
    : DeviceModified(drive)
    , d( new Private( drive ) )
{}

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

    if (d->ptableType == Utils::GPT) {
        s -= (512 * 8 + 16 * 1024 * 8); /* secondary GPT table which replicates the first for safety purposes */
    }

    return s;
}

qulonglong Disk::offset() const
{
    qulonglong off = 0;

    switch (d->ptableType)
    {
        case Utils::MBR: {
            off += 1024 * 1024; /* the first MB is reserved */
            break;
        }

        case Utils::GPT: {
            off += (512 * 8 + 16 * 1024 * 8);
            off += 512 * 8;
            break;
        }

        default:
            break;
    }

    return off;
}

Utils::PTableType Disk::partitionTableScheme() const
{
    return d->ptableType;
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

void Disk::setPartitionTableScheme(Utils::PTableType type)
{
    d->ptableType = type;
}
    
}
}
}
