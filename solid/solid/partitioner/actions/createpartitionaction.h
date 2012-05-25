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
            /**
             * @class CreatePartitionAction
             * @extends Action
             * @brief Action for creating a new partition
             */
            class SOLID_EXPORT CreatePartitionAction : public Action
            {
            public:
                /**
                 * Constructs a new action object.
                 * 
                 * @param disk the name of the disk the partition will be created on.
                 * @param offset the offset (in bytes) of the new partition.
                 * @param size the size (in bytes) of the new partition.
                 * @param ptype the type of the new partition (primary, logical or extended).
                 */
                explicit CreatePartitionAction(const QString& disk,
                                               qulonglong offset,
                                               qulonglong size,
                                               PartitionType ptype
                                              );
                virtual ~CreatePartitionAction();
                
                virtual ActionType actionType() const;
                
                /**
                 * @returns the name of the disk the partition will be created on.
                 */
                QString disk() const;
                
                /**
                 * @returns the offset (in bytes) of the new partition.
                 */
                qulonglong offset() const;
                
                /**
                 * @returns the size (in bytes) of the new partition.
                 */
                qulonglong size() const;
                
                /**
                 * @returns the partition type.
                 */
                PartitionType partitionType() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif