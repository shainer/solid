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
#include <solid/partitioner/utils/filesystemutils.h>
#include <solid/backends/udisks/udisksfilesystem.h>
#include <kglobal.h>
#include <QtCore/QDebug>

#define MEGABYTE 1024*1024

namespace Solid
{
namespace Partitioner
{
namespace Utils
{

using namespace Solid::Backends::UDisks;
    
class FilesystemUtils::Private
{
public:
    Private()
    {}
    
    QHash< QString, KnownFilesystem > filesystems;
    QHash< QString, qulonglong > filesystemMinimumSizes;
};

class FilesystemUtilsHelper
{
public:
    FilesystemUtilsHelper()
        : q(0)
    {}
    
    ~FilesystemUtilsHelper()
    {
        delete q;
    }
    
    
    FilesystemUtils* q;
};

K_GLOBAL_STATIC(FilesystemUtilsHelper, s_filesystemutils);

FilesystemUtils::FilesystemUtils()
    : d( new Private )
{
    Q_ASSERT(!s_filesystemutils->q);
    
    UDisksFilesystem udisksFs;
    d->filesystems = udisksFs.filesystems();
    
    /*
     * FIXME: check these values.
     * These values are in megabytes.
     */
    d->filesystemMinimumSizes.insert("NTFS", 8);
    d->filesystemMinimumSizes.insert("Linux Ext2", 8);
    d->filesystemMinimumSizes.insert("Linux Ext3", 8);
    d->filesystemMinimumSizes.insert("Linux Ext4", 8);
    d->filesystemMinimumSizes.insert("ReiserFS", 32);
    d->filesystemMinimumSizes.insert("FAT", 32);
    
    s_filesystemutils->q = this;
}

FilesystemUtils::~FilesystemUtils()
{
    delete d;
}

FilesystemUtils* FilesystemUtils::instance()
{
    if (!s_filesystemutils->q) {
        new FilesystemUtils;
    }
    
    return s_filesystemutils->q;
}

QStringList FilesystemUtils::supportedFilesystems() const
{
    return d->filesystems.keys();
}

QString FilesystemUtils::filesystemIdFromName(const QString& fsName) const
{
    if (d->filesystems.contains(fsName)) {
        KnownFilesystem fs = d->filesystems[fsName];
        return fs.id;
    }
    
    return QString();
}

QString FilesystemUtils::filesystemNameFromId(const QString& fsId) const
{
    for (QHash< QString, KnownFilesystem >::iterator it = d->filesystems.begin(); it != d->filesystems.end(); it++) {
        KnownFilesystem filesystem = it.value();
        
        if (filesystem.id == fsId) {
            return it.key();
        }
    }
    
    return QString();
}

bool FilesystemUtils::supportsLabel(const QString& filesystem) const
{
    return (filesystemProperty(filesystem, "max_label_len").toUInt() != 0);
}

QVariant FilesystemUtils::filesystemProperty(const QString& fsName, const QString& propertyName) const
{
    if (!d->filesystems.contains(fsName)) {
        return QVariant();
    }
    
    KnownFilesystem filesystem = d->filesystems[fsName];
    QVariant property;
    
    if (propertyName == "name") {
        property = filesystem.name;
    }
    else if (propertyName == "id") {
        property = filesystem.id;
    }
    else if (propertyName == "supports_unix_owners") {
        property = filesystem.supports_unix_owners;
    }
    else if (propertyName == "can_mount") {
        property = filesystem.can_mount;
    }
    else if (propertyName == "can_create") {
        property = filesystem.can_create;
    }
    else if (propertyName == "max_label_len") {
        property = filesystem.max_label_len;
    }
    else if (propertyName == "supports_label_rename") {
        property = filesystem.supports_label_rename;
    }
    else if (propertyName == "supports_online_label_rename") {
        property = filesystem.supports_online_label_rename;
    }
    else if (propertyName == "supports_fsck") {
        property = filesystem.supports_fsck;
    }
    else if (propertyName == "supports_online_fsck") {
        property = filesystem.supports_online_fsck;
    }
    else if (propertyName == "supports_resize_enlarge") {
        property = filesystem.supports_resize_enlarge;
    }
    else if (propertyName == "supports_online_resize_enlarge") {
        property = filesystem.supports_online_resize_enlarge;
    }
    else if (propertyName == "supports_resize_shrink") {
        property = filesystem.supports_resize_shrink;
    }
    else if (propertyName == "supports_online_resize_shrink") {
        property = filesystem.supports_online_resize_shrink;
    }
    
    return property;
}

qulonglong FilesystemUtils::minimumFilesystemSize(const QString& fs)
{
    if (!d->filesystemMinimumSizes.contains(fs)) {
        return 0;
    }
    
    return d->filesystemMinimumSizes[fs] * MEGABYTE;
}

}
}
}