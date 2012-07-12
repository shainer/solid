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
#ifndef SOLID_BACKENDS_UDISKS_H
#define SOLID_BACKENDS_UDISKS_H

#include <QtCore/QString>
#include <QtDBus/QDBusArgument>

namespace Solid
{
    namespace Backends
    {
        namespace UDisks
        {
            /**
             * @struct KnownFilesystem
             * @brief A filesystem supported by UDisks with its properties.
             * 
             * @var id
             * identifier for the filesystem, like vfat or ntfs
             * @var name
             * a more descriptive name
             * @var supports_unix_owners
             * whether it supports the concept of ownerships
             * @var can_mount
             * whether it can be mounted
             * @var can_create
             * whether it can be created on a partition
             * @var max_label_len
             * max label length as unsigned int, or 0 if no label is supported
             * @var supports_label_rename
             * whether the label can be changed
             * @var supports_online_label_rename
             * whether the label can be changed while the fs is mounted
             * @var supports_fsck
             * whether the filesystem can be checked
             * @var supports_online_fsck
             * whether the filesystem can be checked while mounted
             * @var supports_resize_enlarge
             * whether the filesystem's size can be increased
             * @var supports_online_resize_enlarge
             * whether the filesystem's size can be increased while mounted
             * @var supports_resize_shrink
             * whether the filesystem's size can be decreased
             * @var supports_online_resize_shrink
             * whether the filesystem's size can be decreased while mounted
             */
            struct KnownFilesystem
            {
                QString id;
                QString name;
                bool supports_unix_owners;
                bool can_mount;
                bool can_create;
                uint max_label_len;
                bool supports_label_rename;
                bool supports_online_label_rename;
                bool supports_fsck;
                bool supports_online_fsck;
                bool supports_resize_enlarge;
                bool supports_online_resize_enlarge;
                bool supports_resize_shrink;
                bool supports_online_resize_shrink;
            };
            typedef QList<KnownFilesystem> KnownFilesystemList;
            
            /**
             * @class UDisksFilesystem
             * @brief This class retrieves information about the filesystems supported by UDisks.
             */
            class UDisksFilesystem
            {
            public:
                
                /**
                 * Builds a new object. Internally, it communicates with UDisks to get the known filesystems.
                 */
                UDisksFilesystem();
                virtual ~UDisksFilesystem();
                
                /**
                 * @returns a hash of filesystem names -> filesystem structure with properties.
                 */
                QHash< QString, KnownFilesystem > filesystems();
                
            private:
                QHash< QString, KnownFilesystem > m_filesystems;
            };
        }
    }
}

/* These declarations have to be outside all namespaces to work */
Q_DECLARE_METATYPE(Solid::Backends::UDisks::KnownFilesystem);
Q_DECLARE_METATYPE(Solid::Backends::UDisks::KnownFilesystemList);

#endif