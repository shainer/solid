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

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

class ResizePartitionAction::Private
{
public:
    Private(const QString& p, qlonglong o, qlonglong s)
        : partition(p)
        , newOffset(o)
        , newSize(s)
    {}
    
    ~Private()
    {}
    
    QString partition;
    qlonglong newOffset;
    qlonglong newSize;
};
    
ResizePartitionAction::ResizePartitionAction(const QString& partition, qlonglong newOffset, qlonglong newSize)
    : d( new Private(partition, newOffset, newSize) )
{}

ResizePartitionAction::ResizePartitionAction(const QString& partition, qlonglong newSize)
    : d( new Private(partition, -1, newSize) )
{}

ResizePartitionAction::~ResizePartitionAction()
{
    delete d;
}

Action::ActionType ResizePartitionAction::actionType() const
{
    return ResizePartition;
}

QString ResizePartitionAction::description() const
{
    QString desc( "Changing %0 to offset %1 and size %2" );
    desc = desc.arg(d->partition, QString::number(d->newOffset), QString::number(d->newSize));
    
    return QObject::tr(desc.toUtf8().data());
}

QString ResizePartitionAction::partition() const
{
    return d->partition;
}

qlonglong ResizePartitionAction::newOffset() const
{
    return d->newOffset;
}

qlonglong ResizePartitionAction::newSize() const
{
    return d->newSize;
}

}
}
}