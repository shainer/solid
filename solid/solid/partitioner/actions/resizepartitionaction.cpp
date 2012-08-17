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

#include "resizepartitionaction.h"
#include <backends/udisks/udisksdevice.h>

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

class ResizePartitionAction::Private
{
public:
    Private(qulonglong o, qulonglong s, bool b)
        : newOffset(o)
        , newSize(s)
        , safe(b)
    {}
    
    ~Private()
    {}
    
    qlonglong newOffset;
    qlonglong newSize;
    bool safe;
};
    
ResizePartitionAction::ResizePartitionAction(const QString& partition, qulonglong newOffset, qulonglong newSize, bool safe)
    : Action(partition)
    , d( new Private(newOffset, newSize, safe) )
{
    QString desc( "Changing %0 to offset %1 and size %2" );
    QString offsetStr = formatByteSize((double)(d->newOffset));
    QString sizeStr = formatByteSize((double)(d->newSize));
    
    setDescription( desc.arg(partition, offsetStr, sizeStr) );
}

ResizePartitionAction::~ResizePartitionAction()
{
    delete d;
}

Action::ActionType ResizePartitionAction::actionType() const
{
    return ResizePartition;
}

qulonglong ResizePartitionAction::newOffset() const
{
    return d->newOffset;
}

qulonglong ResizePartitionAction::newSize() const
{
    return d->newSize;
}

bool ResizePartitionAction::safe() const
{
    return d->safe;
}

}
}
}