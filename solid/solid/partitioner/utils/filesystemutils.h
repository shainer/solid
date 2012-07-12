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
#ifndef SOLID_PARTITIONER_UTILS_FILESYSTEMUTILS_H
#define SOLID_PARTITIONER_UTILS_FILESYSTEMUTILS_H

#include <solid/solid_export.h>
#include <QtCore/QStringList>
#include <QtCore/QVariant>

namespace Solid
{
    namespace Partitioner
    {
        namespace Utils
        {
            /**
             * @class FilesystemUtils
             * @brief This singleton groups together some utility functions to know about supported filesystems.
             */
            class SOLID_EXPORT FilesystemUtils
            {
                Q_DISABLE_COPY(FilesystemUtils)
                
            public:
                virtual ~FilesystemUtils();
                static FilesystemUtils* instance();
                
                /**
                 * @returns a list of supported filesystems' names.
                 */
                QStringList supportedFilesystems() const;
                
                /**
                 * Retrieves a filesystem identifier by its name.
                 * 
                 * @param fsName a filesystem name
                 * @returns the filesystem identifier.
                 */
                QString filesystemIdFromName(const QString &) const;
                
                /**
                 * Retrieves a filesystem name by its identifier.
                 * 
                 * @param fsId a filesystem identifier.
                 * @returns the filesystem more descriptive name.
                 */
                QString filesystemNameFromId(const QString &) const;
                
                /**
                 * @param fsName a filesystem name.
                 * @returns whether the filesystem supports labels.
                 */
                bool supportsLabel(const QString &) const;
                
                /**
                 * @param fsName a filesystem name.
                 * @param propertyName the name of a filesystem property.
                 * @returns the value of the property as a QVariant, or an invalid object if the property doesn't exist.
                 */
                QVariant filesystemProperty(const QString &, const QString &) const;

                /**
                 * Retrieves the minimum size a partition with this filesystem must have.
                 * 
                 * @param fsName the filesystem name.
                 * @returns the minimum size in bytes.
                 */
                qulonglong minimumFilesystemSize(const QString &);

            private:
                FilesystemUtils();
                
                class Private;
                Private* d;
            };
        }
    }
}

#endif