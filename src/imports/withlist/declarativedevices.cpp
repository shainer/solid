/*
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
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
//#include "declarativedevice.h"

#include <QDebug>

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
        matchingDeviceUdis << device.udi();
        matchingDevices << device;
    }
}

DevicesQueryPrivate::~DevicesQueryPrivate()
{
    handlers.remove(query);
}

void DevicesQueryPrivate::addDevice(const QString &udi)
{
    if (predicate.isValid() && predicate.matches(Solid::Device(udi))) {
        matchingDevices << udi;
        emit deviceAdded(udi);
    }
}

void DevicesQueryPrivate::removeDevice(const QString &udi)
{
    if (predicate.isValid() && matchingDevices.contains(udi)) {
        matchingDevices.removeAll(udi);
        emit deviceRemoved(udi);
    }
}

const QStringList &DevicesQueryPrivate::devices() const
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
    connect(m_backend.data(), &DevicesQueryPrivate::deviceAdded,
            this, &DeclarativeDevices::addDevice);
    connect(m_backend.data(), &DevicesQueryPrivate::deviceRemoved,
            this, &DeclarativeDevices::removeDevice);

    const int matchesCount = m_backend->devices().count();

    if (matchesCount != 0) {
        emit emptyChanged(false);
        emit countChanged(matchesCount);
        emit devicesChanged(m_backend->devices());
    }
}

void DeclarativeDevices::reset()
{
    if (!m_backend) {
        return;
    }

    m_backend->disconnect(this);
    m_backend.reset();

    emit emptyChanged(true);
    emit countChanged(0);
    emit devicesChanged(QStringList());
}

void DeclarativeDevices::addDevice(const QString &udi)
{
    if (!m_backend) {
        return;
    }

    const int count = m_backend->devices().count();

    if (count == 1) {
        emit emptyChanged(false);
    }

    emit countChanged(count);
    emit devicesChanged(m_backend->devices());
    emit deviceAdded(udi);
}

void DeclarativeDevices::removeDevice(const QString &udi)
{
    if (!m_backend) {
        return;
    }

    const int count = m_backend->devices().count();

    if (count == 0) {
        emit emptyChanged(true);
    }

    emit countChanged(count);
    emit devicesChanged(m_backend->devices());
    emit deviceRemoved(udi);
}

DeclarativeDevices::DeclarativeDevices(QObject *parent)
    : QObject(parent)
{
}

DeclarativeDevices::~DeclarativeDevices()
{
}

bool DeclarativeDevices::isEmpty() const
{
    initialize();
    return count() == 0;
}

int DeclarativeDevices::count() const
{
    initialize();
    return devices().count();
}

QStringList DeclarativeDevices::devices() const
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

    emit queryChanged(query);
}

QObject *DeclarativeDevices::device(const QString &udi, const QString &_type)
{
    Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(_type);

    //Solid::DeviceInterface *iface = Solid::Device(udi).asDeviceInterface(type);
    return Solid::Device(udi).asDeviceInterface(type);
    //return new DeclarativeDevice(iface, this);
}

} // namespace Solid
