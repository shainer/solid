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
#ifndef SOLID_PARTITIONER_DEVICES_PARTITION_H
#define SOLID_PARTITIONER_DEVICES_PARTITION_H

#include "devicemodified.h"
#include <solid/partitioner/actions/createpartitionaction.h>
#include <solid/partitioner/utils/partitioner_enums.h>
#include <solid/partitioner/utils/filesystem.h>
#include <solid/storagevolume.h>
#include <unistd.h>
#include <solid/storageaccess.h>
#include <solid/device.h>

namespace Solid
{
    namespace Partitioner
    {        
        namespace Devices
        {
            /**
             * @class Partition
             * @extends DeviceModified
             * 
             * This class represents a partition for the partitioning module.
             */
            class SOLID_EXPORT Partition : public DeviceModified
            {                
            public:
                
                /**
                 * Creates a new partition from an existent one.
                 * 
                 * @param volume the StorageVolume object representing said partition in Solid.
                 */
                explicit Partition(Device &);
                
                /**
                 * Creates a new partition from an application request.
                 * 
                 * @param action the action registered for creating the new partition.
                 */
                explicit Partition(Actions::CreatePartitionAction *);
                explicit Partition(Partition *);
                virtual ~Partition();
                
                DeviceModified::DeviceModifiedType deviceType() const;
                
                /**
                 * @returns true if this partition should be ignored by the application.
                 */
                bool ignored() const;
                
                /**
                 * @returns an enum describing the usage of this partition.
                 */
                StorageVolume::UsageType usage() const;
                
                /**
                 * @returns an object representing the filesystem of this partition, if any.
                 */
                Utils::Filesystem filesystem() const;
                
                /**
                 * @returns the partition label.
                 */
                QString label() const;
                
                /**
                 * @returns the UUID (unique partition ID).
                 */
                QString uuid() const;
                
                /**
                 * @returns the size in bytes.
                 */
                qulonglong size() const;
                
                /**
                 * @returns the initial offset in bytes.
                 */
                virtual qulonglong offset() const;
                
                /**
                 * @returns offset() + size().
                 */
                qulonglong rightBoundary() const;
                
                /**
                 * @returns a value representing the partition type: Primary, Logical or Extended.
                 */
                Utils::PartitionType partitionType() const;
                
                /**
                 * @returns the list of flags set for this partition.
                 */
                QStringList flags() const;
                
                /**
                 * @returns true if the partition is mounted.
                 */
                bool isMounted() const;
                
                /**
                 * @returns the path the partition is mounted on, or an empty string if not mounted.
                 */
                QString mountFile() const;
                
                /**
                 * @returns the StorageAccess object, which can be used to mount or unmount.
                 */
                StorageAccess* access() const;
                
                /**
                 * Sets whether this partition should be displayed by the system.
                 * 
                 * @param ignored whether this partition is to be ignored.
                 */
                void setIgnored(bool);
                
                /**
                 * Sets a new usage for this partition.
                 * 
                 * @param usage an enumeration value for the usage.
                 */
                void setUsage(StorageVolume::UsageType);
                
                /**
                 * Sets a new filesystem.
                 * 
                 * @param fs the object representing the filesystem.
                 */
                void setFilesystem(const Utils::Filesystem &);
                
                /**
                 * Sets a new label.
                 * 
                 * @param label the label.
                 */
                void setLabel(const QString &);
                
                /**
                 * Sets a new size.
                 * 
                 * @param size the new size in bytes.
                 */
                void setSize(qulonglong);
                
                /**
                 * Sets a new initial offset.
                 * 
                 * @param offset the new initial offset in bytes.
                 */
                void setOffset(qulonglong);
                
                /**
                 * Sets a new type for this partition.
                 * 
                 * @param type the new type.
                 */
                void setPartitionType(Utils::PartitionType);
                
                /**
                 * Sets a list of flags to be valid for this partition.
                 * 
                 * @param flags a string list of flags.
                 */
                void setFlags(const QStringList &);

            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
