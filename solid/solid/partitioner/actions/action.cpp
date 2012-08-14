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
#include <solid/partitioner/actions/action.h>
#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

static int actionId = 1;
    
class Action::Private
{
public:
    DeviceModified* owner;
    
    QString description;
    QString uniqueName;
    
    QString partition;
    QString newPartitionName;
};
    
Action::Action()
    : d( new Private )
{}

Action::Action(const QString& partition)
    : d( new Private )
{
    d->partition = partition;
}

Action::~Action()
{
    delete d;
}

bool Action::isPartitionAction() const
{
    return !d->partition.isEmpty();
}

QString Action::partition() const
{
    return d->partition;
}

QString Action::newPartitionName() const
{
    if (d->newPartitionName.isEmpty()) {
        return partition();
    }
    
    return d->newPartitionName;
}

void Action::setNewPartitionName(const QString& newName)
{
    d->newPartitionName = newName;
}

bool Action::operator==(Action *other) const
{
    return uniqueName() == other->uniqueName();
}

QString Action::description() const
{
    return d->description;
}

void Action::setDescription(const QString& desc)
{
    d->description = desc;
    d->uniqueName = d->description + " " + QString::number(actionId++);
}

QString Action::uniqueName() const
{
    return d->uniqueName;
}

DeviceModified* Action::ownerDisk() const
{
    return d->owner;
}

void Action::setOwnerDisk(DeviceModified* owner)
{
    d->owner = owner;
}

} 
}
}