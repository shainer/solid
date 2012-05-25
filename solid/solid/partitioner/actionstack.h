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
#ifndef SOLID_PARTITIONER_ACTIONSTACK_H
#define SOLID_PARTITIONER_ACTIONSTACK_H

#include <QtCore/QList>
#include <solid/partitioner/actions/action.h>

namespace Solid
{
    namespace Partitioner
    {
        using namespace Actions;
    
        /**
         * @class ActionStack
         * @brief Handles the list of actions registered for execution.
         * 
         * Actions are stored in a FIFO storage.
         */
        class ActionStack
        {    
        public:
            /**
             * Constructs an empty action stack.
             */
            ActionStack();
            virtual ~ActionStack();
            
            /**
             * Pushes a new registered action. Previous undone actions are deleted at this point, to avoid
             * out-of-context operations.
             * 
             * @param op the action.
             */
            void push(Action *op);
            
            /**
             * Deletes the last action registered from the list, if one is present, otherwise nothing happens.
             * 
             * @returns the list of "valid" actions, minus the one just undone.
             */
            QList<Action *> undo();
            
            /**
             * Redoes the last action undone, if one is present, otherwise nothing happens.
             * 
             * @returns the list of "valid" actions, including the one just redone.
             */
            QList<Action *> redo();
            
            /**
             * Clears all the list, deleting the action objects.
             */
            void clear();

            /**
             * @returns true if there is no valid action stored, false otherwise.
             */
            bool empty() const;
            
            /**
             * @returns true if there is no undone action, false otherwise.
             */
            bool undoEmpty() const;
            
            /**
             * Looks for an action in the list.
             * 
             * @param action action.
             * @returns true if the action is present among the valid ones, false otherwise.
             */
            bool contains(Action *) const;
            
            /**
             * @returns the list of valid actions.
             */
            QList<Action *> list() const;
            
            /**
             * @returns the list of undone actions.
             */
            QList<Action *> undoList() const;
            
        private:
            class Private;
            Private* d;
        };
    
    }

}

#endif