/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *   Copyright (C) 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This library is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU Lesser General Public
 *   License as published by the Free Software Foundation; either
 *   version 2.1 of the License, or (at your option) any later version.
 *
 *   This library is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *   Lesser General Public License for more details.
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "declarativedevices.h"
#include "declarativedevices_p.h"

#include <QDebug>
#include <QQmlEngine>

#include <solid/device.h>
#include <solid/deviceinterface.h>
#include <solid/devicenotifier.h>
#include <solid/genericinterface.h>

namespace Solid
{

// Maps queries to the handler objects
QHash<QString, QWeakPointer<DevicesQueryPrivate> > DevicesQueryPrivate::handlers;

QSharedPointer<DevicesQueryPrivate> DevicesQueryPrivate::forQuery(const QString &query)
{
    if (handlers.contains(query)) {
        return handlers[query].toStrongRef();
    }

    // Creating a new shared backend instance
    QSharedPointer<DevicesQueryPrivate> backend(new DevicesQueryPrivate(query));

    // Storing a weak pointer to the backend
    handlers[query] = backend;

    // Returns the newly created backend
    // TODO: It would be nicer with std::move and STL's smart pointers,
    // but RVO should optimize this out.
    return backend;
}

DevicesQueryPrivate::DevicesQueryPrivate(const QString &query)
    : query(query)
    , predicate(Solid::Predicate::fromString(query))
    , notifier(Solid::DeviceNotifier::instance())
{
    connect(notifier, &Solid::DeviceNotifier::deviceAdded,
            this,     &DevicesQueryPrivate::addDevice);
    connect(notifier, &Solid::DeviceNotifier::deviceRemoved,
            this,     &DevicesQueryPrivate::removeDevice);

    if (!query.isEmpty() && !predicate.isValid()) {
        return;
    }

    Q_FOREACH (const Solid::Device &device, Solid::Device::listFromQuery(predicate)) {
        QObject *deviceInterface = deviceInterfaceFromUdi(device.udi());
        if (deviceInterface) {
            matchingDevices << deviceInterface;
            connect(deviceInterface, &QObject::destroyed, this, &DevicesQueryPrivate::deviceDestroyed);
        }
    }
}

DevicesQueryPrivate::~DevicesQueryPrivate()
{
    handlers.remove(query);
}

#define return_conditionally_SOLID_DEVICE_INTERFACE(Type, Device) \
    if (Device.isDeviceInterface(Type)) { \
        QObject* obj = Device.asDeviceInterface(Type); \
        obj->setParent(this);\
        return obj; \
    }

QObject *DevicesQueryPrivate::deviceInterfaceFromUdi(const QString &udi)
{
    Solid::Device device(udi);

    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::Processor, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::Block, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::StorageAccess, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::StorageDrive, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::OpticalDrive, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::StorageVolume, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::OpticalDisc, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::Camera, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::PortableMediaPlayer, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::Battery, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::NetworkShare, device);
    return_conditionally_SOLID_DEVICE_INTERFACE(Solid::DeviceInterface::GenericInterface, device);

    return 0;
}

void DevicesQueryPrivate::addDevice(const QString &udi)
{
    if (predicate.isValid() && predicate.matches(Solid::Device(udi))) {
        QObject *device = deviceInterfaceFromUdi(udi);
        if (device) {
            // Needed otherwise QML kills it when it's done
            QQmlEngine::setObjectOwnership(device, QQmlEngine::CppOwnership);
            matchingDevices << device;
            connect(device, &QObject::destroyed, this, &DevicesQueryPrivate::deviceDestroyed);
            Q_EMIT deviceAdded(device);
        }
    }
}

void DevicesQueryPrivate::removeDevice(const QString &udi)
{
    Q_EMIT deviceRemoved(udi);
}

void DevicesQueryPrivate::deviceDestroyed(QObject *obj)
{
    const int index = matchingDevices.indexOf(obj);
    Q_EMIT aboutToRemoveDeviceFromModel(index);
    if (index == -1) {
        return;
    }
    matchingDevices.removeAt(index);
    Q_EMIT removedDeviceFromModel(index);
}

QList<QObject *> DevicesQueryPrivate::devices() const
{
    return matchingDevices;
}

void DeclarativeDevices::initialize() const
{
    if (m_backend) {
        return;
    }

    m_backend = DevicesQueryPrivate::forQuery(m_query);

    connect(m_backend.data(), &DevicesQueryPrivate::deviceAdded,
            this, &DeclarativeDevices::addDevice);
    connect(m_backend.data(), &DevicesQueryPrivate::deviceRemoved,
            this, &DeclarativeDevices::removeDevice);
    connect(m_backend.data(), &DevicesQueryPrivate::aboutToRemoveDeviceFromModel,
            this, &DeclarativeDevices::aboutToRemoveDeviceFromModel);
    connect(m_backend.data(), &DevicesQueryPrivate::removedDeviceFromModel,
            this, &DeclarativeDevices::removeDeviceFromModel);

    Q_EMIT rowCountChanged(m_backend->devices().count());
}

void DeclarativeDevices::reset()
{
    if (!m_backend) {
        return;
    }

    beginResetModel();
    m_backend->disconnect(this);
    m_backend.clear();
    endResetModel();

    Q_EMIT rowCountChanged(0);
}

void DeclarativeDevices::addDevice(const QObject *device)
{
    if (!m_backend) {
        return;
    }

    const int count = m_backend->devices().count();

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    endInsertRows();

    Q_EMIT rowCountChanged(count);
    Q_EMIT deviceAdded(device);
}

void DeclarativeDevices::removeDevice(const QString &udi)
{
    if (!m_backend) {
        return;
    }

    const int count = m_backend->devices().count();

    Q_EMIT rowCountChanged(count);
    Q_EMIT deviceRemoved(udi);
}

void DeclarativeDevices::aboutToRemoveDeviceFromModel(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
}

void DeclarativeDevices::removeDeviceFromModel(int)
{
    endRemoveRows();
}

DeclarativeDevices::DeclarativeDevices(QAbstractListModel *parent)
    : QAbstractListModel(parent)
{
}

DeclarativeDevices::~DeclarativeDevices()
{
}

int DeclarativeDevices::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    initialize();
    return m_backend->devices().count();
}

QVariant DeclarativeDevices::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount()) {
        return QVariant();
    }

    if (role == DeviceRole) {
        return QVariant::fromValue(m_backend->devices().at(index.row()));
    }
    return QVariant();
}

QObject *DeclarativeDevices::get(const int index) const
{
    return m_backend->devices().at(index);
}

QList<QObject *> DeclarativeDevices::devices() const
{
    initialize();
    return m_backend->devices();
}

QString DeclarativeDevices::query() const
{
    return m_backend->query;
}

void DeclarativeDevices::setQuery(const QString &query)
{
    if (m_query == query) {
        return;
    }

    m_query = query;

    reset();
    initialize();

    Q_EMIT queryChanged(query);
}

QHash<int, QByteArray> DeclarativeDevices::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DeviceRole] = "device";
    return roles;
}

} // namespace Solid
