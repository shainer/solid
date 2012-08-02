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

#include <solid/partitioner/devices/devicemodified.h>
#include <solid/partitioner/utils/partitiontableutils.h>
#include <solid/partitioner/utils/partitioner_enums.h>
#include <solid/device.h>

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
                 * Builds a new disk from system information.
                 * 
                 * @param dev the Device object.
                 */
                explicit Disk(Device);
                
                /**
                 * Builds a new disk object with all properties set to their default values.
                 * This is used just temporarily to copy a device.
                 */
                explicit Disk();
                
                /**
                 * Destructor.
                 */
                virtual ~Disk();
                
                /**
                 * @returns a DeviceModifieType enum value. For this kind of object, it's always DiskDevice.
                 */
                virtual DeviceModifiedType deviceType() const;
                
                /**
                 * @returns a dynamically allocated copy of this disk.
                 */
                virtual DeviceModified* copy() const;
                
                /**
                 * @returns the size of the disk in bytes.
                 */
                virtual qulonglong size() const;
                
                /**
                 * @returns the initial offset in bytes, that means the first byte available for creating partitions
                 * after the information of the partition table.
                 */
                virtual qulonglong offset() const;
                
                /**
                 * @returns offset() + size()
                 */
                virtual qulonglong rightBoundary() const;
                
                /**
                 * @returns the partition table scheme, or an empty string if there isn't one.
                 */
                virtual QString partitionTableScheme() const;
                
                /**
                 * @returns the minimum size a partition can have on this disk. Geometry constraints force partitions to be
                 * at least of size equals to the minimum between 1MB and 1% of the disk size.
                 */
                virtual qulonglong minimumPartitionSize() const;
                
                /**
                 * Sets a new name for the disk. This extends the already existing method in DeviceModified because
                 * when we know the disk's name we can obtain its sector size (which is needed by other methods).
                 * 
                 * @param name the new name.
                 */
                virtual void setName(const QString &);
                
                /**
                 * Sets a size.
                 * 
                 * @note the partitioning module cannot change the size of a disk. This method doesn't do anything.
                 * @param size the new size in bytes
                 */
                virtual void setSize(qulonglong);
                
                /**
                 * Sets a new initial offset.
                 * 
                 * @note the partitioning module cannot change the offset of a disk. This method doesn't do anything.
                 * @param the new offset in bytes.
                 */
                virtual void setOffset(qulonglong);
                
                /**
                 * Sets a new partition table scheme for the disk.
                 * 
                 * @param scheme the new scheme name, empty for none.
                 */
                virtual void setPartitionTableScheme(const QString &);

            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
