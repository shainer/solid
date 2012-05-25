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
             * @brief Action to modify label and/or flags of a partition.
             * 
             * Why do we have three constructors? This action can be used to modify label, flags or both.
             * But an empty list for flags can mean "we don't want to change the flags" or "the partition had the flag boot
             * set and now we unset it" (taking the flag "boot" as an example). The same applies to the label (new
             * label empty vs. the same label as before).
             * 
             * To avoid this ambiguation and provide a simple API to the application, there are three different
             * constructors (and internal flags to remember what the application requested).
             */
            class ModifyPartitionAction : public Action
            {
            public:
                explicit ModifyPartitionAction(const QString& partition,
                                               const QString& label,
                                               const QStringList& flags);
                explicit ModifyPartitionAction(const QString& partition,
                                               const QStringList& flags);
                explicit ModifyPartitionAction(const QString& partition,
                                               const QString& label);
                virtual ~ModifyPartitionAction();
                
                virtual ActionType actionType() const;
                virtual QString description() const;
                
                QString partition() const;
                QString label() const;
                
                bool bootable() const;
                bool required() const;
                
                bool isLabelChanged() const;
                bool isFlagChanged() const;
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif