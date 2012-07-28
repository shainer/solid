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
    Private(const Filesystem& fs)
        : filesystem(fs)
    {}
    
    ~Private()
    {}
    
    void setNames()
    {
        QString fsName = filesystem.name();
        QString fsLabel = filesystem.label();
        QString ownerUid = QString::number( filesystem.ownerUid() );
        QString ownerGid = QString::number( filesystem.ownerGid() );
        
        QString desc( "Changing filesystem of partition %0 to %1" );
        desc = desc.arg(q->partition(), fsName);
        
        if (!fsLabel.isEmpty()) {
            desc.append( QString("; label %0").arg(fsLabel) );
        }
        
        if (ownerUid != "-1") {
            desc.append( QString("; ownerUid %0").arg(ownerUid)  );
        }
        if (ownerGid != "-1") {
            desc.append( QString("; ownerGid %0").arg(ownerGid) );
        }
        
        q->setDescription(desc);
    }
    
    FormatPartitionAction* q;
    Filesystem filesystem;
};
    
FormatPartitionAction::FormatPartitionAction(const QString& partition, const Filesystem& fs)
    : Action(partition)
    , d( new Private(fs) )
{
    d->q = this;
    d->setNames();
}

FormatPartitionAction::FormatPartitionAction(const QString& partition, const QString& fsName)
    : Action(partition)
    , d( new Private(Filesystem(fsName)) )
{
    d->q = this;
    d->setNames();
}

FormatPartitionAction::~FormatPartitionAction()
{
    delete d;
}

Action::ActionType FormatPartitionAction::actionType() const
{
    return Action::FormatPartition;
}

Filesystem FormatPartitionAction::filesystem() const
{
    return d->filesystem;
}

}
}
}
