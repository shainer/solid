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

#ifndef SOLID_PARTITIONER_ACTIONS_CREATEPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_CREATEPARTITIONACTION_H

#include <solid/partitioner/actions/action.h>
#include <solid/partitioner/utils/partitioner_enums.h>
#include <QtCore/QStringList>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            /**
             * @class CreatePartitionAction
             * @extends Action
             * @brief Action for creating a new partition
             */
            class SOLID_EXPORT CreatePartitionAction : public Action
            {
            public:
                /**
                 * Constructs a new action object.
                 * 
                 * @param disk the name of the disk the partition will be created on.
                 * @param offset the offset (in bytes) of the new partition.
                 * @param size the size (in bytes) of the new partition.
                 * @param ptype the type of the new partition (primary, logical or extended).
                 * @param label the partition label.
                 * @param flags partition flags.
                 */
                explicit CreatePartitionAction(const QString& disk,
                                               qulonglong offset,
                                               qulonglong size,
                                               Utils::PartitionType ptype,
                                               const QString& label = QString(),
                                               const QStringList& flags = QStringList()
                                              );
                
                /**
                 * See the constructor above.
                 * 
                 * This is a convenience method for GPT partitions, where there is no concept of
                 * Primary, Extended or Logical. Any information the application provides in this sense
                 * is ignored.
                 */
                explicit CreatePartitionAction(const QString& disk,
                                               qulonglong offset,
                                               qulonglong size,
                                               const QString& label = QString(),
                                               const QStringList& flags = QStringList()
                                              );
                
                virtual ~CreatePartitionAction();
                
                virtual ActionType actionType() const;
                virtual QString description() const;
                
                /**
                 * @returns the name of the disk the partition will be created on.
                 */
                QString disk() const;
                
                /**
                 * @returns the offset (in bytes) of the new partition.
                 */
                qulonglong offset() const;
                
                /**
                 * @returns the size (in bytes) of the new partition.
                 */
                qulonglong size() const;
                
                /**
                 * @returns the partition type.
                 */
                Utils::PartitionType partitionType() const;
                
                /**
                 * @returns the partition label.
                 */
                QString label() const;
                
                /**
                 * @returns the list of partition flags.
                 */
                QStringList flags() const;

            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif
