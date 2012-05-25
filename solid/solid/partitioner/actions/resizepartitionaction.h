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

#ifndef SOLID_PARTITIONER_ACTIONS_RESIZEPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_RESIZEPARTITIONACTION_H

#include "action.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class ResizePartitionAction
             * @extends Action
             * @brief A class to resize and/or move a partition.
             */
            class SOLID_EXPORT ResizePartitionAction : public Action
            {
            public:
                /**
                 * Constructors.
                 * 
                 * @param partition the partition to modify.
                 * @param newOffset the new offset, in bytes.
                 * @param newSize the new size, in bytes.
                 * 
                 * Passing -1 means you don't want the offset or size to change.
                 * If you use the second constructor the new offset is automatically set to -1.
                 */
                explicit ResizePartitionAction(const QString& partition, qlonglong newOffset, qlonglong newSize = -1);
                explicit ResizePartitionAction(const QString& partition, qlonglong newSize);
                virtual ~ResizePartitionAction();
                
                virtual ActionType actionType() const;
                virtual QString description() const;
                
                /**
                 * @returns the partition to modify.
                 */
                QString partition() const;
                
                /**
                 * @returns the new size, in bytes.
                 */
                qlonglong newSize() const;
                
                /**
                 * @returns the new offset, in bytes.
                 */
                qlonglong newOffset() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif