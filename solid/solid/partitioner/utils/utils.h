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

#ifndef SOLID_PARTITIONER_UTILS_UTILS_H
#define SOLID_PARTITIONER_UTILS_UTILS_H

#define SPACE_BETWEEN_LOGICALS 32256
#define LOOPDEVICE_MAJOR           7
#define MEGABYTE               1204 * 1204

#include <QtCore/QString>
#include <solid/solid_export.h>
#include <solid/partitioner/volumetree.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Utils
        {            
            /**
             * Finds all blocks of free space inside a disk.
             * 
             * @param diskTree the tree representing the disk layout.
             * @returns a list of free space blocks.
             */
            QList< Devices::FreeSpace*> freeSpaceOfDisk(const VolumeTree &);
            
            /**
             * Retrieves the size of a disk.
             * 
             * @param udi the disk UDI.
             * @returns the disk size in bytes.
             */
            qulonglong getDiskSize(const QString &);
        }
    }
}

#endif