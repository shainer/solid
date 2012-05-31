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
#ifndef SOLID_PARTITIONER_DEVICES_STORAGEDRIVEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_STORAGEDRIVEMODIFIED_H

#include "devicemodified.h"
#include <solid/storagedrive.h>
#include <solid/partitioner/utils/partitiontableutils.h>
#include <solid/partitioner/utils/partitioner_enums.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {
            /**
             * @class Disk
             * @extends DeviceModified
             * 
             * This class represents a disk for the partitioning module.
             */
            class SOLID_EXPORT Disk : public DeviceModified
            {
            public:
                
                /**
                 * Builds a new disk from a StorageDrive object.
                 * 
                 * @param drive the StorageDrive.
                 */
                explicit Disk(StorageDrive *);
                virtual ~Disk();
                
                DeviceModifiedType deviceType() const;
                
                /**
                 * @returns the size of the disk in bytes.
                 */
                qulonglong size() const;
                
                /**
                 * @returns the initial offset in bytes, that means the first byte available for creating partitions
                 * after the information of the partition table.
                 */
                qulonglong offset() const;
                
                /**
                 * @returns offset() + size()
                 */
                qulonglong rightBoundary() const;
                
                /**
                 * @returns an enum value for the ptable scheme, or None if there isn't a table.
                 */
                Utils::PTableType partitionTableScheme() const;
                
                /**
                 * Sets a size.
                 * 
                 * @note the partitioning module cannot change the size of a disk. This method doesn't do anything.
                 * @param size the new size in bytes
                 */
                void setSize(qulonglong);
                
                /**
                 * Sets a new initial offset.
                 * 
                 * @note the partitioning module cannot change the offset of a disk. This method doesn't do anything.
                 * @param the new offset in bytes.
                 */
                void setOffset(qulonglong);
                
                /**
                 * Sets a new partition table scheme for the disk.
                 * 
                 * @param scheme the new scheme.
                 */
                void setPartitionTableScheme(Utils::PTableType);

            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
