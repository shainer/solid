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
#include <QtCore/QString>
#include <solid/solid_export.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Utils
        {
            /**
             * Converts from a device UDI detected by udisks, like /org/freedesktop/UDisks/devices/sda, to the
             * correspondent Linux-like name, e.g. /dev/sda.
             * 
             * @param udi the device UDI to convert.
             * @returns the Linux-like device name.
             * @note not sure if this conversion is universally valid.
             */
            QString SOLID_EXPORT udiToName(const QString &);
            
            /**
             * Converts from a Linux-like device name to a device UDI.
             * 
             * @param name the Linux-like device name.
             * @returns the correspondent device UDI.
             * @note not sure if this conversion is universally valid.
             */
            QString SOLID_EXPORT nameToUdi(const QString &);
        }
    }
}

#endif