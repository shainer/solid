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
#include <solid/partitioner/actions/partitionaction.h>

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

class PartitionAction::Private
{
public:
    Private(const QString& p)
        : partition(p)
    {}
    
    ~Private()
    {}
    
    QString partition;
    QString newPartitionName;
};

PartitionAction::PartitionAction(const QString& partition)
    : d( new Private(partition) )
{}

PartitionAction::~PartitionAction()
{
    delete d;
}

QString PartitionAction::partition() const
{
    return d->partition;
}

QString PartitionAction::newPartitionName() const
{
    if (d->newPartitionName.isEmpty()) { /* if no name has been assigned, just use the normal one */
        return d->partition;
    }
    
    return d->newPartitionName;
}

void PartitionAction::setNewPartitionName(const QString& name)
{
    d->newPartitionName = name;
}

}
}
}