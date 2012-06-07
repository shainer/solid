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
#ifndef SOLID_PARTITIONER_ACTIONS_PARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_PARTITIONACTION_H

#include <solid/solid_export.h>
#include <solid/partitioner/actions/action.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class PartitionAction
             * @extends Action
             * @author Lisa Vitolo <shainer@chakra-project.org>
             * 
             * This class is an abstract base class for all the actions that operate on a partition
             * (CreatePartitionAction isn't included).
             */
            class SOLID_EXPORT PartitionAction : public Action
            {
            public:
                explicit PartitionAction(const QString& partition);
                virtual ~PartitionAction();
                
                /**
                 * Retrieves the type of the object.
                 * 
                 * @returns the most derived class a generic Action object is instance of.
                 */
                virtual Action::ActionType actionType() const = 0;
                
                /**
                 * @returns a localized string depicting the action.
                 */
                virtual QString description() const = 0;
                
                /**
                 * @returns the partition involved in the action.
                 */
                virtual QString partition() const;
                
                /**
                 * Sets a different partition name for this action.
                 * 
                 * @param name the new partition name.
                 */
                virtual void setPartitionName(const QString &);
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif