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

#ifndef SOLID_PARTITIONER_ACTIONS_MODIFYFILESYSTEMACTION_H
#define SOLID_PARTITIONER_ACTIONS_MODIFYFILESYSTEMACTION_H
#include <solid/partitioner/actions/action.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class ModifyFilesystemAction
             * @extends Action
             * @brief This class allows to change the filesystem properties (for now, only the label) without reformatting.
             */
            class SOLID_EXPORT ModifyFilesystemAction : public Action
            {
            public:
                /**
                 * Builds a new object.
                 * 
                 * @param partition the partition with the filesystem to modify.
                 * @param label the new filesystem label.
                 */
                explicit ModifyFilesystemAction(const QString &, const QString &);
                virtual ~ModifyFilesystemAction();
                
                /**
                 * @returns ActionType::ChangeFilesystem.
                 */
                virtual ActionType actionType() const;

                /**
                 * @returns the new filesystem label.
                 */
                QString fsLabel() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif