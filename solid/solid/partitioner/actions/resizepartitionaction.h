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

#include <solid/partitioner/actions/action.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class ResizePartitionAction
             * @extends Action
             * @author Lisa Vitolo <shainer@chakra-project.org>
             * @brief A class to resize and/or move a partition.
             */
            class SOLID_EXPORT ResizePartitionAction : public Action
            {
            public:
                /**
                 * Constructor.
                 * 
                 * @param partition the partition to modify.
                 * @param newOffset the new offset, in bytes.
                 * @param newSize the new size, in bytes.
                 * @param safe whether the resizing should preserve the filesystem and data, if present. Default to true.
                 * 
                 * @warning setting safe to true means the action can be denied if safe resizing isn't possible for some reason.
                 */
                explicit ResizePartitionAction(const QString& partition,
                                               qulonglong newOffset,
                                               qulonglong newSize,
                                               bool safe = true);
                virtual ~ResizePartitionAction();
                
                /**
                 * @returns ActionType::ResizePartition
                 */
                virtual ActionType actionType() const;
                
                /**
                 * @returns the new size, in bytes.
                 */
                qulonglong newSize() const;
                
                /**
                 * @returns the new offset, in bytes.
                 */
                qulonglong newOffset() const;
                
                /**
                 * @returns whether safe resizing should be performed.
                 */
                bool safe() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif