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
#ifndef SOLID_PARTITIONER_ACTIONS_REMOVEPARTITIONTABLEACTION_H
#define SOLID_PARTITIONER_ACTIONS_REMOVEPARTITIONTABLEACTION_H
#include "action.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class RemovePartitionTableAction
             * @extends Action
             * @author Lisa Vitolo <shainer@chakra-project.org>
             * @brief Action to delete a partition table from a disk.
             * @warning executing this action causes the automatic removal of all partitions present on the disk.
             */
            class SOLID_EXPORT RemovePartitionTableAction : public Action
            {
            public:
                /**
                 * Creates a new object.
                 * 
                 * @param disk the disk name.
                 */
                explicit RemovePartitionTableAction(const QString &);
                virtual ~RemovePartitionTableAction();
                
                /**
                 * @returns ActionType::RemovePartitionTable
                 */
                virtual ActionType actionType() const;
                
                /**
                 * @returns the disk name.
                 */
                QString disk() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif