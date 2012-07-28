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
#ifndef SOLID_PARTITIONER_ACTIONEXECUTER_H
#define SOLID_PARTITIONER_ACTIONEXECUTER_H

#include <solid/solid_export.h>
#include <solid/partitioner/actions/action.h>
#include <solid/partitioner/volumetree.h>
#include <solid/partitioner/partitioningerror.h>
#include "volumetreemap.h"

#include <QtCore/QObject>

namespace Solid
{
    namespace Partitioner
    {
        /**
         * @class ActionExecuter
         * @extends QObject
         * @author Lisa Vitolo <shainer@chakra-project.org>
         * @brief This class manages the execution on the system of a list of actions.
         */
        class ActionExecuter : public QObject
        {
            Q_OBJECT
            
        public:
            /**
             * Builds a new object.
             * 
             * @param actions the list of actions to apply.
             * @param map the map containing disk layouts.
             */
            ActionExecuter(const QList< Actions::Action* > &, const VolumeTreeMap &);
            virtual ~ActionExecuter();
            
            /**
             * Returns whether this is a valid executor.
             * 
             * @returns true if we got the access to apply actions, false otherwise.
             */
            bool isValid() const;
            
            /**
             * Executes on system.
             * 
             * @returns true if the execution was successful, false otherwise.
             */
            bool execute();
            
            /**
             * @returns the latest error occurred.
             */
            PartitioningError error() const;
            
        signals:
            
            /**
             * This is emitted when an action's execution completes successfully.
             * 
             * @param actionNumber the position of the action in the list, starting from 1.
             */
            void nextActionCompleted(int);
            
        private:
            class Private;
            Private* d;
        };
    
    }
    
}

#endif