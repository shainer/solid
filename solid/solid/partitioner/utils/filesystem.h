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
#ifndef SOLID_PARTITIONER_DEVICES_FILESYSTEM_H
#define SOLID_PARTITIONER_DEVICES_FILESYSTEM_H

#include <unistd.h>
#include <QtCore/QStringList>
#include <solid/solid_export.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Utils
        {
            /**
             * @class Filesystem
             * @brief This class represents a filesystem with its properties.
             */
            class SOLID_EXPORT Filesystem
            {
            public:
                /**
                 * Creates a new filesystem.
                 *
                 * @param name the filesystem name.
                 * @param label the filesystem label, where supported.
                 * @param uid the owner UID, if ownership is supported.
                 * @param gid the owner GID, if ownership is supported.
                 */
                explicit Filesystem(const QString &name,
                                    const QString &label,
                                    uid_t ownerUid,
                                    gid_t ownerGid);

                /**
                 * Creates a new filesystem.
                 *
                 * @param name the filesystem name.
                 * @param flags the filesystem flags: each string has the format "name=value".
                 */
                explicit Filesystem(const QString &name,
                                    const QStringList &flags = QStringList());


                /**
                 * Creates a void filesystem. It means "No filesystem".
                 */
                explicit Filesystem();

                /**
                 * Copy constructor.
                 */
                Filesystem(const Filesystem &);
                virtual ~Filesystem();
                
                /**
                 * @returns the filesystem name.
                 */
                QString name() const;

                /**
                 * @returns the filesystem label.
                 */
                QString label() const;

                /**
                 * @returns the owner UID; -1 means either no UID was set or this filesystem doesn't support
                 * owernship.
                 */
                uid_t ownerUid() const;

                /**
                 * @returns the owner GID; -1 means either no GID was set or this filesystem doesn't support
                 * ownership.
                 */
                gid_t ownerGid() const;
                
                /**
                 * @returns if the string list was used to pass flags, a list of those flags who weren't
                 * recognized: either they were wrong in some part or this filesystem doesn't support them.
                 */
                QStringList unsupportedFlags() const;
                
            private:
                class Private;
                Private* d;
            };
        }
    }
}

#endif
