/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010 Lukas Tinkl <ltinkl@redhat.com>

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

#include "udisksmanager.h"
#include "udisks.h"

#include <QtDBus/QDBusReply>
#include <QtCore/QDebug>
#include <QtDBus/QDBusMetaType>
#include <QtDBus/QDBusConnectionInterface>

#include "../shared/rootdevice.h"
#include <partitioner/devices/disk.h>
#include <solid/partitioner/utils/utils.h>

using namespace Solid::Backends::UDisks;
using namespace Solid::Backends::Shared;
using namespace Solid::Partitioner;
using namespace Solid::Partitioner::Devices;

const QDBusArgument &operator>>(const QDBusArgument &argument, FileSystems &knownFS)
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

QDBusArgument &operator<<(QDBusArgument &argument, const FileSystems &knownFS)
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

const QDBusArgument &operator>>(const QDBusArgument &argument, FileSystemsList &knownFSList)
{
    argument.beginArray();
    knownFSList.clear();

    while(!argument.atEnd()) {
        FileSystems element;
        argument >> element;
        knownFSList.append(element);
    }

    argument.endArray();
    return argument;
}

QDBusArgument &operator<<(QDBusArgument &argument, const FileSystemsList &knownFSList)
{
    argument.beginArray(qMetaTypeId<FileSystems>());
    for(int i = 0; i < knownFSList.count(); ++i) {
        argument << knownFSList[i];
    }

    argument.endArray();

    return argument;
}

UDisksManager::UDisksManager(QObject *parent)
    : Solid::Ifaces::DeviceManager(parent),
      m_manager(UD_DBUS_SERVICE,
                UD_DBUS_PATH,
                UD_DBUS_INTERFACE_DISKS,
                QDBusConnection::systemBus())
{
    m_supportedInterfaces
            << Solid::DeviceInterface::GenericInterface
            << Solid::DeviceInterface::Block
            << Solid::DeviceInterface::StorageAccess
            << Solid::DeviceInterface::StorageDrive
            << Solid::DeviceInterface::OpticalDrive
            << Solid::DeviceInterface::OpticalDisc
            << Solid::DeviceInterface::StorageVolume
            << Solid::DeviceInterface::FreeSpace;

    qDBusRegisterMetaType<QList<QDBusObjectPath> >();
    qDBusRegisterMetaType<QVariantMap>();

    bool serviceFound = m_manager.isValid();
    if (!serviceFound) {
        // find out whether it will be activated automatically
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                                                              "/org/freedesktop/DBus",
                                                              "org.freedesktop.DBus",
                                                              "ListActivatableNames");

        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid() && reply.value().contains(UD_DBUS_SERVICE)) {
            QDBusConnection::systemBus().interface()->startService(UD_DBUS_SERVICE);
            serviceFound = true;
        }
    }

    if (serviceFound) {
        connect(&m_manager, SIGNAL(DeviceAdded(QDBusObjectPath)),
                this, SLOT(slotDeviceAdded(QDBusObjectPath)));
        connect(&m_manager, SIGNAL(DeviceRemoved(QDBusObjectPath)),
                this, SLOT(slotDeviceRemoved(QDBusObjectPath)));
        connect(&m_manager, SIGNAL(DeviceChanged(QDBusObjectPath)),
                this, SLOT(slotDeviceChanged(QDBusObjectPath)));
    }
}

UDisksManager::~UDisksManager()
{

}

QObject* UDisksManager::createDevice(const QString& udi)
{
    if (udi==udiPrefix()) {
        RootDevice *root = new RootDevice(udi);

        root->setProduct(tr("Storage"));
        root->setDescription(tr("Storage devices"));
        root->setIcon("server-database"); // Obviously wasn't meant for that, but maps nicely in oxygen icon set :-p

        return root;

    } else if (deviceCache().contains(udi)) {
        return new UDisksDevice(udi);

    } else {
        return 0;
    }
}

QStringList UDisksManager::devicesFromQuery(const QString& parentUdi, Solid::DeviceInterface::Type type)
{
    QStringList result;

    if (!parentUdi.isEmpty())
    {
        foreach (const QString &udi, deviceCache())
        {
            UDisksDevice device(udi);
            if (device.queryDeviceInterface(type) && device.parentUdi() == parentUdi)
                result << udi;
        }

        return result;
    }
    else if (type != Solid::DeviceInterface::Unknown)
    {
        foreach (const QString &udi, deviceCache())
        {
            UDisksDevice device(udi);
            if (device.queryDeviceInterface(type))
                result << udi;
        }

        return result;
    }

    return deviceCache();
}

QStringList UDisksManager::allDevices()
{
    m_knownDrivesWithMedia.clear();
    m_deviceCache.clear();
    m_deviceCache << udiPrefix();

    foreach(const QString &udi, allDevicesInternal())
    {
        m_deviceCache.append(udi);

        UDisksDevice device(udi);
        if (device.queryDeviceInterface(Solid::DeviceInterface::OpticalDrive)) // forge a special (separate) device for optical discs
        {
            if (device.prop("DeviceIsOpticalDisc").toBool())
            {
                if (!m_knownDrivesWithMedia.contains(udi))
                    m_knownDrivesWithMedia.append(udi);
                m_deviceCache.append(udi + ":media");
            }
        }
    }

    return m_deviceCache;
}

QList< FreeSpace* > UDisksManager::freeSpaceOfDisk(const Solid::Partitioner::VolumeTree& diskTree)
{
    QList< VolumeTreeItem* > primaryPartitions = diskTree.rootNode()->children();
    Disk* disk = dynamic_cast<Disk *>(diskTree.root());
    VolumeTreeItem* extended = diskTree.extendedNode();
    
    QList< FreeSpace* > freeSpaces;
    
    freeSpaces.append( findSpace(primaryPartitions, disk) );
    
    if (extended) {
        freeSpaces.append( findSpace(extended->children(), extended->volume(), true) );
    }
    
    return freeSpaces;
}

QList< FreeSpace* > UDisksManager::findSpace(QList< VolumeTreeItem* > partitions, DeviceModified* parent, bool logicals)
{
    QList< FreeSpace* > freeSpaces;
    
    if (partitions.isEmpty()) {
        freeSpaces.append( new FreeSpace(parent->offset(), parent->size(), parent->name()) );
        return freeSpaces;
    }
    
    for (int i = -1; i < partitions.size(); i++) {
        Partition* volume1 = NULL;
        Partition* volume2 = NULL;
        FreeSpace *space = NULL;
        
        if (i == -1) {
            volume2 = dynamic_cast<Partition *>(partitions[i+1]->volume());
        }
        else if (i < partitions.size() - 1) {
            volume1 = dynamic_cast<Partition *>(partitions[i]->volume());
            volume2 = dynamic_cast<Partition *>(partitions[i+1]->volume());
        }
        else {
            volume1 = dynamic_cast<Partition *>(partitions[i]->volume());
        }
        
        if (!logicals) {
            space = spaceBetweenPartitions(volume1, volume2, parent);
        } else {
            space = spaceBetweenLogicalPartitions(volume1, volume2, parent);
        }
        
        if (space) {
            freeSpaces.append(space);
        }
    }
    
    return freeSpaces;
}

FreeSpace* UDisksManager::spaceBetweenPartitions(Partition* partition1,
                                                 Partition* partition2,
                                                 DeviceModified* disk)
{
    FreeSpace* sp = 0;
    
    if (!partition1) {
        qulonglong initialOffset = disk->offset();
        if (partition2->offset() > initialOffset) {
            sp = new FreeSpace(initialOffset, partition2->offset() - initialOffset, disk->name());
        }
    }
    else if (!partition2) {
        if (partition1->rightBoundary() < disk->size()) {
            sp = new FreeSpace(partition1->rightBoundary(), disk->size() - partition1->rightBoundary(), disk->name());
        }
    }
    else {
        if (partition1->rightBoundary() < partition2->offset()) {
            sp = new FreeSpace(partition1->rightBoundary(), (partition2->offset() - partition1->rightBoundary()), disk->name());
        }
    }
    
    return sp;
}

FreeSpace* UDisksManager::spaceBetweenLogicalPartitions(Partition* partition1,
                                                        Partition* partition2,
                                                        DeviceModified* extended)
{
    FreeSpace* sp = 0;
    
    if (!partition1) {
        qulonglong e_offset = extended->offset() + SPACE_BETWEEN_LOGICALS;
        if (e_offset < partition2->offset()) {
            sp = new FreeSpace(e_offset, partition2->offset() - e_offset, extended->name());
        }
    }
    else if (!partition2) {
        qulonglong e_rightOffset = extended->offset() + extended->size();
        if (partition1->rightBoundary() < e_rightOffset) {
            sp = new FreeSpace(partition1->rightBoundary(), e_rightOffset - partition1->rightBoundary(), extended->name());
        }
    }
    else {
        if (partition1->rightBoundary() + SPACE_BETWEEN_LOGICALS < partition2->offset()) {
            sp = new FreeSpace(partition1->rightBoundary(), partition2->offset() - partition1->rightBoundary(), extended->name());
        }
    }
    
    return sp;
}

QStringList UDisksManager::allDevicesInternal()
{
    QDBusReply<QList<QDBusObjectPath> > reply = m_manager.call("EnumerateDevices");

    if (!reply.isValid()) {
        qWarning() << Q_FUNC_INFO << " error: " << reply.error().name();
        return QStringList();
    }

    QStringList retList;
    foreach(const QDBusObjectPath &path, reply.value()) {
        retList << path.path();
    }

    return retList;
}

QSet< Solid::DeviceInterface::Type > UDisksManager::supportedInterfaces() const
{
    return m_supportedInterfaces;
}

QStringList UDisksManager::supportedFilesystems() const
{
    qRegisterMetaType<FileSystems>("Filesystems");
    qDBusRegisterMetaType<FileSystems>();
    qDBusRegisterMetaType<FileSystemsList>();
    
    QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.UDisks.Introspectable",
                                                            "/org/freedesktop/UDisks",
                                                            "org.freedesktop.UDisks",
                                                            "Introspect");
    QDBusReply<QString> reply = QDBusConnection::systemBus().call(message);
    qDebug() << reply.value();
    
    FileSystemsList fslist = m_manager.property("KnownFilesystems").value<FileSystemsList>();
    
    foreach (FileSystems fs, fslist) {
        qDebug() << fs.name << fs.id;
    }
    
    return QStringList();
}

QString UDisksManager::udiPrefix() const
{
    return UD_UDI_DISKS_PREFIX;
}

void UDisksManager::slotDeviceAdded(const QDBusObjectPath &opath)
{
    const QString udi = opath.path();

    if (!m_deviceCache.contains(udi)) {
        m_deviceCache.append(udi);
    }

    UDisksDevice device(udi);
    if (device.queryDeviceInterface(Solid::DeviceInterface::StorageDrive)
            && !device.prop("DeviceIsMediaAvailable").toBool()
            && !m_dirtyDevices.contains(udi))
        m_dirtyDevices.append(udi);

    emit deviceAdded(udi);
    slotDeviceChanged(opath);  // case: hotswap event (optical drive with media inside)
}

void UDisksManager::slotDeviceRemoved(const QDBusObjectPath &opath)
{
    const QString udi = opath.path();

    // case: hotswap event (optical drive with media inside)
    if (m_knownDrivesWithMedia.contains(udi)) {
        m_knownDrivesWithMedia.removeAll(udi);
        m_deviceCache.removeAll(udi + ":media");
        emit deviceRemoved(udi + ":media");
    }

    if (m_dirtyDevices.contains(udi))
        m_dirtyDevices.removeAll(udi);

    emit deviceRemoved(udi);
    m_deviceCache.removeAll(opath.path());
}

void UDisksManager::slotDeviceChanged(const QDBusObjectPath &opath)
{
    const QString udi = opath.path();
    UDisksDevice device(udi);

    if (device.queryDeviceInterface(Solid::DeviceInterface::OpticalDrive))
    {
        if (!m_knownDrivesWithMedia.contains(udi) && device.prop("DeviceIsOpticalDisc").toBool())
        {
            m_knownDrivesWithMedia.append(udi);
            if (!m_deviceCache.isEmpty()) {
                m_deviceCache.append(udi + ":media");
            }
            emit deviceAdded(udi + ":media");
        }

        if (m_knownDrivesWithMedia.contains(udi) && !device.prop("DeviceIsOpticalDisc").toBool())
        {
            m_knownDrivesWithMedia.removeAll(udi);
            m_deviceCache.removeAll(udi + ":media");
            emit deviceRemoved(udi + ":media");
        }
    }

    if (device.queryDeviceInterface(Solid::DeviceInterface::StorageDrive)
            && device.prop("DeviceIsMediaAvailable").toBool()
            && m_dirtyDevices.contains(udi))
    {
        //qDebug() << "dirty device added:" << udi;
        emit deviceAdded(udi);
        m_dirtyDevices.removeAll(udi);
    }
}

const QStringList &UDisksManager::deviceCache()
{
    if (m_deviceCache.isEmpty())
        allDevices();

    return m_deviceCache;
}


#include "backends/udisks/udisksmanager.moc"
