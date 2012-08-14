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
#ifndef SOLID_PARTITIONER_ACTIONS_MODIFYPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_MODIFYPARTITIONACTION_H

#include <solid/partitioner/actions/action.h>
#include <QtCore/QStringList>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class ModifyPartitionAction
             * @extends Action
             * @author Lisa Vitolo <shainer@chakra-project.org>
             * @brief Action to modify label and/or flags of a partition.
             */
            class SOLID_EXPORT ModifyPartitionAction : public Action
            {
            public:
                
                /**
                 * Creates an action object.
                 * 
                 * @param partition the partition to be changed.
                 * @param label the new label.
                 * @param flags new flags for this partition.
                 */
                explicit ModifyPartitionAction(const QString& partition,
                                               const QString& label,
                                               const QStringList& flags = QStringList());
                
                /**
                 * Creates an action object.
                 * This can be used when you want the label to stay the same without telling it explicitly.
                 * 
                 * @param partition the partition to be changed.
                 * @param flags a list of flags to set.
                 */
                explicit ModifyPartitionAction(const QString& partition,
                                               const QStringList& flags);
                virtual ~ModifyPartitionAction();
                
                /**
                 * @returns ActionType::ModifyPartition
                 */
                virtual ActionType actionType() const;
                
                /**
                 * @returns the new label.
                 */
                QString label() const;
                
                /**
                 * @returns the list of flags to set.
                 */
                QStringList flags() const;
                
                /**
                 * @returns true if the user requested to change the label.
                 */
                bool isLabelChanged() const;
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif
