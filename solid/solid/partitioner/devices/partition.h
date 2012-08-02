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

#include <solid/partitioner/devices/devicemodified.h>
#include <solid/partitioner/actions/createpartitionaction.h>
#include <solid/partitioner/utils/partitioner_enums.h>
#include <solid/partitioner/filesystem.h>
#include <solid/storagevolume.h>
#include <kauthactionreply.h>
#include <solid/storageaccess.h>
#include <solid/device.h>

namespace Solid
{
    namespace Partitioner
    {        
        namespace Devices
        {
            using namespace KAuth;
            
            /**
             * @class Partition
             * @extends DeviceModified
             * 
             * This class represents a partition for the partitioning module.
             */
            class SOLID_EXPORT Partition : public QObject, public DeviceModified
            {                
                Q_OBJECT
                
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
                 * @param scheme the partition table scheme on which the partition is created
                 */
                explicit Partition(Actions::CreatePartitionAction *, const QString &);
                
                /**
                 * Builds a new partition object with all properties set to their default values.
                 * This is used just temporarily to copy a device.
                 */
                explicit Partition();
                
                /**
                 * Destructor.
                 */
                virtual ~Partition();
                
                /**
                 * @returns a DeviceModifieType enum value. For this kind of object, it's always PartitionDevice.
                 */
                virtual DeviceModified::DeviceModifiedType deviceType() const;
                
                /**
                 * @returns a dynamically allocated copy of this partition.
                 */
                virtual DeviceModified* copy() const;
                
                /**
                 * @returns true if this partition should be ignored by the application.
                 * @see StorageVolume::isIgnored()
                 */
                virtual bool ignored() const;
                
                /**
                 * @returns an enum describing the usage of this partition.
                 */
                virtual StorageVolume::UsageType usage() const;
                
                /**
                 * @returns an object representing the filesystem of this partition, if any.
                 */
                virtual Filesystem filesystem() const;
                
                /**
                 * @returns the partition label.
                 */
                virtual QString label() const;
                
                /**
                 * @returns the UUID (unique partition ID).
                 */
                virtual QString uuid() const;
                
                /**
                 * @returns the size in bytes.
                 */
                virtual qulonglong size() const;
                
                /**
                 * Retrieves the minimum size this partition can be resized to. This is 0 unless the partition has a filesystem
                 * for which we support resizing: in that case, the minimum size is the minimum this partition can have without
                 * deleting any data.
                 * 
                 * @returns the partition's minimum size, in bytes.
                 */
                virtual qulonglong minimumSize() const;
                
                /**
                 * @returns the initial offset in bytes.
                 */
                virtual qulonglong offset() const;
                
                /**
                 * @returns offset() + size().
                 */
                virtual qulonglong rightBoundary() const;
                
                /**
                 * @returns a value representing the partition type: Primary, Logical or Extended.
                 */
                virtual Utils::PartitionType partitionType() const;
                
                /**
                 * @returns the string describing this partition's type.
                 * An example are the GUID string used in GPT tables.
                 */
                virtual QString partitionTypeString() const;
                
                /**
                 * @returns the ptable scheme this partition is part of. 
                 */
                virtual QString partitionTableScheme() const;
                
                /**
                 * @returns the list of flags set for this partition.
                 */
                virtual QStringList flags() const;
                
                /**
                 * @returns true if the partition is mounted.
                 */
                virtual bool isMounted() const;
                
                /**
                 * @returns the path the partition is mounted on, or an empty string if not mounted.
                 */
                virtual QString mountFile() const;
                
                /**
                 * @returns the StorageAccess object, which can be used to mount or unmount.
                 */
                virtual StorageAccess* access() const;
                
                /**
                 * @returns whether this partition can be resized safely
                 */
                virtual bool supportsResizing() const;
                
                /**
                 * Resizes this partition.
                 * 
                 * @param newSize the new partition's size.
                 * @returns whether the resizing succeeded
                 */
                virtual bool resize(qulonglong);
                
                /**
                 * Sets whether this partition should be displayed by the system.
                 * 
                 * @param ignored whether this partition is to be ignored.
                 */
                virtual void setIgnored(bool);
                
                /**
                 * Sets a new usage for this partition.
                 * 
                 * @param usage an enumeration value for the usage.
                 */
                virtual void setUsage(StorageVolume::UsageType);
                
                /**
                 * Sets a new filesystem.
                 * 
                 * @param fs the object representing the filesystem.
                 */
                virtual void setFilesystem(const Filesystem &);
                
                /**
                 * Sets a new label.
                 * 
                 * @param label the label.
                 */
                virtual void setLabel(const QString &);
                
                /**
                 * Sets a new size.
                 * 
                 * @param size the new size in bytes.
                 */
                virtual void setSize(qulonglong);
                
                /**
                 * Sets a new initial offset.
                 * 
                 * @param offset the new initial offset in bytes.
                 */
                virtual void setOffset(qulonglong);
                
                /**
                 * Sets a new type for this partition.
                 * 
                 * @param type the new type.
                 */
                virtual void setPartitionType(Utils::PartitionType);
                
                /**
                 * Sets a new type string for this partition.
                 * 
                 * @param type the new type string.
                 * @note this is currently used just for copying, as you cannot always change it (when you can,
                 * you can just use format).
                 */
                virtual void setPartitionTypeString(const QString &);
                
                /**
                 * Sets the ptable scheme this partition is part of.
                 * 
                 * @param scheme the ptable scheme.
                 */
                virtual void setPartitionTableScheme(const QString &);
                
                /**
                 * Sets a list of flags to be valid for this partition.
                 * 
                 * @param flags a string list of flags.
                 */
                virtual void setFlags(const QStringList &);
                
                /**
                 * Sets the access interface used to get information related to mounting volumes.
                 * This is actually only used internally to simplify copies.
                 * 
                 * @param access the Solid::StorageAccess interface.
                 */
                virtual void setAccess(StorageAccess *);
                
                /**
                 * Calls the resize helper to retrieve the partition's minimum size.
                 * 
                 * @internal
                 */
                virtual void setMinimumSize();

            private:
                class Private;
                Private* d;
                
            private slots:
                void doAccessibilityChanged(bool accessible, const QString &udi);
                void minimumSizeReady(ActionReply);
            };
            
        }
    }
}

#endif
