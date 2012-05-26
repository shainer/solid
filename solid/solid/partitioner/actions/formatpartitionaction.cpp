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

#include "formatpartitionaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{
    
class FormatPartitionAction::Private
{
public:
    Private(const QString &p, const Utils::Filesystem& fs)
        : partition(p)
        , filesystem(fs)
    {}
    
    ~Private()
    {}
    
    QString partition;
    Utils::Filesystem filesystem;
};
    
FormatPartitionAction::FormatPartitionAction(const QString& partition, const Utils::Filesystem& fs)
    : d( new Private(partition, fs) )
{}

FormatPartitionAction::~FormatPartitionAction()
{
    delete d;
}

Action::ActionType FormatPartitionAction::actionType() const
{
    return Action::FormatPartition;
}

QString FormatPartitionAction::description() const
{
    QString desc( "Changing filesystem of partition %0 to %1 with label %2, owner uid %3, owner gid %4" );
    desc = desc.arg(d->partition,
                    d->filesystem.name(),
                    d->filesystem.label(),
                    QString::number(d->filesystem.ownerUid()),
                    QString::number(d->filesystem.ownerGid()));
    
    return QObject::tr(desc.toUtf8().data());
}


QString FormatPartitionAction::partition() const
{
    return d->partition;
}

Utils::Filesystem FormatPartitionAction::filesystem() const
{
    return d->filesystem;
}


}
}
}
