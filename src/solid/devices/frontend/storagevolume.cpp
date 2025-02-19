/*
    Copyright 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include "storagevolume.h"
#include "storagevolume_p.h"

#include "soliddefs_p.h"
#include <solid/devices/ifaces/storagevolume.h>
#include <solid/device.h>

Solid::StorageVolume::StorageVolume(QObject *backendObject)
    : DeviceInterface(*new StorageVolumePrivate(), backendObject)
{
}

Solid::StorageVolume::StorageVolume(StorageVolumePrivate &dd, QObject *backendObject)
    : DeviceInterface(dd, backendObject)
{
}

Solid::StorageVolume::~StorageVolume()
{

}

bool Solid::StorageVolume::isIgnored() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), true, isIgnored());
}

Solid::StorageVolume::UsageType Solid::StorageVolume::usage() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), Unused, usage());
}

QString Solid::StorageVolume::fsType() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), fsType());
}

QString Solid::StorageVolume::label() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), label());
}

QString Solid::StorageVolume::uuid() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), QString(), uuid().toLower());
}

qulonglong Solid::StorageVolume::size() const
{
    Q_D(const StorageVolume);
    return_SOLID_CALL(Ifaces::StorageVolume *, d->backendObject(), 0, size());
}

Solid::Device Solid::StorageVolume::encryptedContainer() const
{
    Q_D(const StorageVolume);

    Ifaces::StorageVolume *iface
        = qobject_cast<Ifaces::StorageVolume *>(d->backendObject());

    if (iface != 0) {
        return Device(iface->encryptedContainerUdi());
    } else {
        return Device();
    }
}

