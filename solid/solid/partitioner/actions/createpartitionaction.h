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

#ifndef SOLID_PARTITIONER_ACTIONS_CREATEPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_CREATEPARTITIONACTION_H

#include <solid/partitioner/actions/action.h>
#include <solid/partitioner/partitioner_enums.h>
#include <QtCore/QString>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {           
            class SOLID_EXPORT CreatePartitionAction : public Action
            {
            public:
                explicit CreatePartitionAction(const QString& disk,
                                               qulonglong offset,
                                               qulonglong size,
                                               PartitionType ptype
                                              );
                virtual ~CreatePartitionAction();
                
                virtual ActionType actionType() const;
                QString disk() const;
                qulonglong offset() const;
                qulonglong size() const;
                PartitionType partitionType() const;
                
            private:
                QString m_disk;
                qulonglong m_offset;
                qulonglong m_size;
                PartitionType m_partitionType;
            };
        }
    }
}

#endif