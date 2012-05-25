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
#include "createpartitionaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{
    
class CreatePartitionAction::Private
{
public:
    Private(const QString &d, qulonglong o, qulonglong s, PartitionType t)
        : disk(d)
        , offset(o)
        , size(s)
        , partitionType(t)
    {}
    
    ~Private()
    {}
    
    QString disk;
    qulonglong offset;
    qulonglong size;
    PartitionType partitionType;
};

CreatePartitionAction::CreatePartitionAction(const QString& disk,
                                             qulonglong offset,
                                             qulonglong size,
                                             PartitionType ptype)
    : d( new Private(disk, offset, size, ptype) )
{}

CreatePartitionAction::~CreatePartitionAction()
{
    delete d;
}

Action::ActionType CreatePartitionAction::actionType() const
{
    return Action::CreatePartition;
}

QString CreatePartitionAction::disk() const
{
    return d->disk;
}

qulonglong CreatePartitionAction::offset() const
{
    return d->offset;
}

qulonglong CreatePartitionAction::size() const
{
    return d->size;
}

PartitionType CreatePartitionAction::partitionType() const
{
    return d->partitionType;
}
    
}
}
}