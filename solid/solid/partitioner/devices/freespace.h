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
#ifndef SOLID_PARTITIONER_DEVICES_FREESPACE_H
#define SOLID_PARTITIONER_DEVICES_FREESPACE_H

#include "devicemodified.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {
            /**
             * @class FreeSpace
             * @extends DeviceModified
             * 
             * This class represents a free space block inside a disk.
             */
            class SOLID_EXPORT FreeSpace : public DeviceModified
            {
            public:
                /**
                 * Creates a new free space block.
                 * 
                 * @param offset the initial offset in bytes.
                 * @param size the size in bytes.
                 * @param parentUdi the udi of the disk this block is placed into.
                 */
                explicit FreeSpace(qulonglong, qulonglong, const QString&);
                virtual ~FreeSpace();
                
                virtual DeviceModifiedType deviceType() const;
                
                /**
                 * @returns the size in bytes.
                 */
                qulonglong size() const;
                
                /**
                 * @returns the initial offset in bytes.
                 */
                qulonglong offset() const;
                
                /**
                 * @returns offset() + size().
                 */
                qulonglong rightBoundary() const;
                
                /**
                 * Most partitioning libraries don't allow the creation of partitions with size less than the minimum between
                 * 1MB and 1% of the disk size, for alignment reasons. The block still needs to exist in the system, but the
                 * application cannot create a new partition on it.
                 * 
                 * @return whether this free space block surpasses the minimum size required from creation.
                 */
                bool isMinimumSize() const;
                
                /**
                 * Sets a new size for this block.
                 * 
                 * @param size the new size in bytes.
                 */
                void setSize(qulonglong);
                
                /**
                 * Sets a new offset for this block.
                 * 
                 * @param offset the new offset in bytes.
                 */
                void setOffset(qulonglong);
                
            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif