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
#ifndef SOLID_PARTITIONER_ACTIONS_FORMATPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_FORMATPARTITIONACTION_H

#include "action.h"
#include <solid/partitioner/utils/filesystem.h>
#include <QtCore/QStringList>
#include <unistd.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class FormatPartitionAction
             * @extends Action
             * @brief Action to format a partition with a new filesystem.
             */
            class SOLID_EXPORT FormatPartitionAction : public Action
            {
            public:
                /**
                 * Constructs a new object.
                 * 
                 * @param partition the partition to format.
                 * @param fs the new filesystem.
                 */
                explicit FormatPartitionAction(const QString &, const Utils::Filesystem &);
                virtual ~FormatPartitionAction();
                
                virtual Action::ActionType actionType() const;
                virtual QString description() const;
                
                /**
                 * @returns the partition to be formatted.
                 */
                QString partition() const;
                
                /**
                 * @returns an object representing the new filesystem.
                 */
                Utils::Filesystem filesystem() const;

            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif
