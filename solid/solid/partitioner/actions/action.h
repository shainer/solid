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

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class Action
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
                    ResizePartition
                };
                
                explicit Action() {}
                virtual ~Action() {}
                
                /**
                 * Retrieves the type of the object.
                 * 
                 * @returns the most derived class a generic Action object is instance of.
                 */
                virtual Action::ActionType actionType() const = 0;
            };
        }
    }
}

#endif
