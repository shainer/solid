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
#ifndef SOLID_PARTITIONER_ACTIONS_CREATEPARTITIONTABLEACTION_H
#define SOLID_PARTITIONER_ACTIONS_CREATEPARTITIONTABLEACTION_H

#include "action.h"
#include <solid/partitioner/utils/partitioner_enums.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class CreatePartitionTableAction
             * @extends Action
             * @brief Action to create a new partition table on a disk.
             * 
             * @warning executing this action causes the automatic removal of all partitions present on the disk.
             */
            class SOLID_EXPORT CreatePartitionTableAction : public Action
            {
            public:
                /**
                 * Creates a new object.
                 * 
                 * @param disk the name of the disk.
                 * @param scheme an identifier of the new partition table scheme to be created.
                 */
                explicit CreatePartitionTableAction(const QString &, Utils::PTableType);
                virtual ~CreatePartitionTableAction();

                virtual ActionType actionType() const;
                virtual QString description() const;

                /**
                 * @returns the name of disk to change.
                 */
                QString disk() const;
                
                /**
                 * @returns the chosen partition table scheme.
                 */
                Utils::PTableType partitionTableScheme() const;

            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif
