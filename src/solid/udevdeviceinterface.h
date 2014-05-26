/*
    Copyright 2014 Kai Uwe Broulik <kde@privat.broulik.de>

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

#ifndef SOLID_BACKENDDEVICEINTERFACE_H
#define SOLID_BACKENDDEVICEINTERFACE_H

#include <solid/ifaces/deviceinterface.h>
#include <solid/ifaces/device.h>

#include <QtCore/QObject>
#include <QtCore/QStringList>

namespace Solid
{
class BackendDeviceInterface : public QObject, virtual public Solid::Ifaces::DeviceInterface
{
    Q_OBJECT
    Q_INTERFACES(Solid::Ifaces::DeviceInterface)
public:
    BackendDeviceInterface(Solid::Ifaces::Device *device, QObject *parent = 0);
    virtual ~BackendDeviceInterface();

    QString udi() const;
    QString parentUdi() const;
    QString vendor() const;
    QString product() const;
    QString icon() const;
    QStringList emblems() const;
    QString description() const;

protected:
    Solid::Ifaces::Device *m_device;
};
}

#endif // SOLID_BACKENDDEVICEINTERFACE_H
