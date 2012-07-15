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
#include "createpartitiontableaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

class CreatePartitionTableAction::Private
{
public:
    Private(const QString& d, Utils::PartitionTableScheme t)
        : disk(d)
        , scheme(t)
    {
    
        switch (scheme) {
            case Utils::MBRScheme: {
                schemeName = "mbr";
                break;
            }
            
            case Utils::GPTScheme: {
                schemeName = "gpt";
                break;
            }
            
            case Utils::APMScheme: {
                schemeName = "apm";
                break;
            }
            
            default: {
                schemeName = "none";
                break;
            }
        }
    }

    ~Private()
    {}

    QString disk;
    Utils::PartitionTableScheme scheme;
    QString schemeName;
};

CreatePartitionTableAction::CreatePartitionTableAction(const QString &disk, Utils::PartitionTableScheme type)
    : d( new Private(disk, type) )
{
    QString desc = "Creating partition table of type %0 on %1";
    desc = desc.arg( d->schemeName, d->disk );
    
    setDescription(desc);
}

CreatePartitionTableAction::~CreatePartitionTableAction()
{
    delete d;
}

Action::ActionType CreatePartitionTableAction::actionType() const
{
    return CreatePartitionTable;
}

QString CreatePartitionTableAction::disk() const
{
    return d->disk;
}

Utils::PartitionTableScheme CreatePartitionTableAction::partitionTableScheme() const
{
    return d->scheme;
}

QString CreatePartitionTableAction::schemeName() const
{
    return d->schemeName;
}

}
}
}
