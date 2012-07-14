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
#ifndef SOLID_PARTITIONER_ACTIONS_ACTION_H
#define SOLID_PARTITIONER_ACTIONS_ACTION_H

#include <solid/solid_export.h>
#include <QtCore/QString>
#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <solid/partitioner/devices/devicemodified.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            using namespace Devices;
            
            /**
             * @class Action
             * @author Lisa Vitolo <shainer@chakra-project.org>
             * @brief This is the abstract base class for all the actions the application can register for execution.
             * 
             * Please note that no error checking is performed while you create the action. All errors are detected
             * when the action is registered for execution.
             */
            class SOLID_EXPORT Action
            {
            public:
                
                /**
                 * @enum ActionType
                 * @brief Enum with a value for each derived class.
                 */
                enum ActionType {
                    CreatePartition,
                    RemovePartition,
                    FormatPartition,
                    ResizePartition,
                    ModifyPartition,
                    CreatePartitionTable,
                    RemovePartitionTable
                };
                
                explicit Action();
                virtual ~Action();
                
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
                 * Comparison operator.
                 * 
                 * @returns true if two actions have the same description.
                 */
                virtual bool operator==(Action *) const;
                
                /**
                 * Sets the "owner disk", i.e. the disk on which this action is to be performed.
                 * 
                 * @param dev the owner disk.
                 */
                virtual void setOwnerDisk(DeviceModified *);
                
                /**
                 * @returns the owner disk of this action.
                 * @see setOwnerDisk().
                 */
                virtual DeviceModified* ownerDisk() const;
                
                /**
                 * Sets the "opposite action" of this one. The opposite action is the one that undoes all the changes
                 * introduced by this one.
                 * 
                 * @param a the opposite action.
                 */
                virtual void setOppositeAction(Action *);
                
                /**
                 * @returns the opposite action of this one.
                 * @see setOppositeAction().
                 */
                virtual Action* oppositeAction() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif
