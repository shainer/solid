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
#include <solid/backends/udisks/udisksfilesystem.h>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusConnection>
#include <QtCore/QDebug>

namespace Solid
{
namespace Backends
{
namespace UDisks
{

inline const QDBusArgument &operator>>(const QDBusArgument &argument, KnownFilesystem &knownFS)
{
    argument.beginStructure();

    argument >> knownFS.id >> knownFS.name >> knownFS.supports_unix_owners >> knownFS.can_mount >> knownFS.can_create >> knownFS.max_label_len
             >> knownFS.supports_label_rename >> knownFS.supports_online_label_rename
             >> knownFS.supports_fsck >> knownFS.supports_online_fsck
             >> knownFS.supports_resize_enlarge >> knownFS.supports_online_resize_enlarge
             >> knownFS.supports_resize_shrink >> knownFS.supports_resize_shrink;

    argument.endStructure();

    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const KnownFilesystem &knownFS)
{
    argument.beginStructure();

    argument << knownFS.id << knownFS.name << knownFS.supports_unix_owners << knownFS.can_mount << knownFS.can_create << knownFS.max_label_len
             << knownFS.supports_label_rename << knownFS.supports_online_label_rename
             << knownFS.supports_fsck << knownFS.supports_online_fsck
             << knownFS.supports_resize_enlarge << knownFS.supports_online_resize_enlarge
             << knownFS.supports_resize_shrink << knownFS.supports_resize_shrink;

    argument.endStructure();

    return argument;
}

inline const QDBusArgument &operator>>(const QDBusArgument &argument, KnownFilesystemList &knownFSList)
{
    argument.beginArray();
    knownFSList.clear();

    while (!argument.atEnd()) {
        KnownFilesystem element;
        argument >> element;
        knownFSList.append(element);
    }

    argument.endArray();
    return argument;
}

inline QDBusArgument &operator<<(QDBusArgument &argument, const KnownFilesystemList &knownFSList)
{
    argument.beginArray(qMetaTypeId<Solid::Backends::UDisks::KnownFilesystem>());
    for (int i = 0; i < knownFSList.count(); ++i) {
        argument << knownFSList[i];
    }

    argument.endArray();

    return argument;
}
    
UDisksFilesystem::UDisksFilesystem()
{
    qDBusRegisterMetaType<Solid::Backends::UDisks::KnownFilesystem>();
    qDBusRegisterMetaType<Solid::Backends::UDisks::KnownFilesystemList>();

    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.UDisks",
                                                          "/org/freedesktop/UDisks",
                                                          QLatin1String("org.freedesktop.DBus.Properties"),
                                                          QLatin1String("Get"));

    QList<QVariant> arguments;
    arguments << "org.freedesktop.UDisks" << "KnownFilesystems";
    message.setArguments(arguments);

    QDBusMessage reply = QDBusConnection::systemBus().call(message);
    QList<QVariant> args = reply.arguments();

    foreach (QVariant arg, args)
    {
        KnownFilesystemList fsList;
        arg.value<QDBusVariant>().variant().value<QDBusArgument>() >> fsList;

        foreach (KnownFilesystem fs, fsList) {
            m_filesystems.insert(fs.name, fs);
        }

    }
}

UDisksFilesystem::~UDisksFilesystem()
{}

QHash< QString, KnownFilesystem > UDisksFilesystem::filesystems()
{
    return m_filesystems;
}

}
}
}