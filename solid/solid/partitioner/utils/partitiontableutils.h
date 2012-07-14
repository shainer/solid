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
#ifndef SOLID_PARTITIONER_UTILS_PARTITIONTABLEUTILS_H
#define SOLID_PARTITIONER_UTILS_PARTITIONTABLEUTILS_H

#include <QtCore/QHash>
#include <QtCore/QStringList>

#include <solid/partitioner/utils/partitioner_enums.h>
#include <solid/solid_export.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Utils
        {
            /**
             * @class PartitionTableUtils
             * @brief This class includes some utility methods about partition tables
             */
            class SOLID_EXPORT PartitionTableUtils
            {
                Q_DISABLE_COPY(PartitionTableUtils)
                
            public:
                virtual ~PartitionTableUtils();
                static PartitionTableUtils* instance();

                /**
                 * Retrieves the supported partition flags for each scheme.
                 * 
                 * @param scheme the name of the table scheme.
                 * @returns a list of accepted flags.
                 */
                QStringList supportedFlags(const QString &);
                
                /**
                 * Converts a partition type to the appropriate string.
                 * 
                 * @param scheme the scheme name.
                 * @param type the type.
                 * 
                 * @returns the correspondent type string.
                 */
                QString typeString(const QString &, QString);
                
            private:
                PartitionTableUtils();

                class Private;
                Private* d;
            };

        }
    }

}

#endif // PARTITIONTABLE_H
