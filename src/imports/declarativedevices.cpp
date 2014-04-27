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
        }
    }
}

DevicesQueryPrivate::~DevicesQueryPrivate()
{
    handlers.remove(query);
}

#define return_conditionally_SOLID_DEVICE_INTERFACE(Type, Device) \
    if (Device.isDeviceInterface(Type)) { \
        return Device.asDeviceInterface(Type); \
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
            matchingDevices << device;
            connect(device, &QObject::destroyed, this, &DevicesQueryPrivate::removeDevice);
            emit deviceAdded(device);
        }
    }
}

void DevicesQueryPrivate::removeDevice(const QString &udi)
{
    qDebug() << "REMOVE DEVICE";
    /*
    sender();
    matchingDevices.removeAll(ponter)
    if (predicate.isValid()) {
        for(int i = 0; i < matchingDevices.count(); ++i) {
            qDebug() << i << matchingDevices.count();
            if (!matchingDevices.at(i)) {
                qDebug() << "this isnt there anymore";
                continue;
            }
            qDebug() << "IS there?" << matchingDevices.at(i);

            qDebug() << "Blah" << matchingDevices.at(i)->property("vendor");
            qDebug() << "Found" << matchingDevices.at(i)->property("udi");
            /*continue;
            if (matchingDevices.at(i) && matchingDevices.at(i)->property("udi") == udi) {
                qDebug() << "REMOVING THIS ONE";
                matchingDevices.removeAt(i);
                //emit deviceRemoved(udi, i);
                //matchingDevices.at(i)->deleteLater();
                break;
            }*/
/*        }
    }*/
}

void DevicesQueryPrivate::removeDevice(QObject *device)
{

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

    const int matchesCount = m_backend->devices().count();
    qDebug() << "Init found" << matchesCount << "matches";

    if (matchesCount != 0) {
        emit emptyChanged(false);
        //emit Changed(matchesCount);
        //emit devicesChanged(m_backend->devices());
    }
}

void DeclarativeDevices::reset()
{
    qDebug() << "I AM ABOUT TO RESET MZ POINTERS AND CRASH";
    if (!m_backend) {
        return;
    }

    m_backend->disconnect(this);
    m_backend.reset();

    emit emptyChanged(true);
    emit rowCountChanged(0);
    //emit devicesChanged(QStringList());
}

void DeclarativeDevices::addDevice(const QObject *device)
{
    if (!m_backend) {
        return;
    }

    const int count = m_backend->devices().count();

    if (count == 1) {
        emit emptyChanged(false);
    }

    const int insertItemIndex = qMax(0, rowCount() - 1);
    beginInsertRows(QModelIndex(), insertItemIndex, insertItemIndex);
    endInsertRows();

    emit rowCountChanged(count);
    emit deviceAdded(device);
}

void DeclarativeDevices::removeDevice(const QString &udi, const int index)
{
    if (!m_backend) {
        return;
    }

    const int count = m_backend->devices().count();

    if (count == 0) {
        emit emptyChanged(true);
    }

    beginRemoveRows(QModelIndex(), index, index);
    endRemoveRows();

    emit rowCountChanged(count);
    emit deviceRemoved(udi);
}

DeclarativeDevices::DeclarativeDevices(QAbstractListModel *parent)
    : QAbstractListModel(parent)
{
}

DeclarativeDevices::~DeclarativeDevices()
{
}

bool DeclarativeDevices::isEmpty() const
{
    initialize();
    return rowCount() == 0;
}

int DeclarativeDevices::rowCount(const QModelIndex &parent) const
{
    //qDebug() << "row count" << this;

    initialize();
    /*qDebug() << "Row count requested";
    qDebug() << m_backend;
    qDebug() << m_backend->devices();*/
    /*qDebug() << devices();
    qDebug() << devices().count();*/

    //qDebug() << "Counting rows";
    //qDebug() << m_backend->devices();

    Q_UNUSED(parent);
    return m_backend->devices().count();
}

QVariant DeclarativeDevices::data(const QModelIndex &index, int role) const
{
    //qDebug() << "Accessing data of row " << index.row();
    if (index.row() < 0 || index.row() >= rowCount()) {
        qDebug() << "Out of bounds";
        return QVariant();
    }

    if (role == Qt::DisplayRole) {
        return QString("display role"); // FIXME
    } else if (role == DeviceRole) {
       /*qDebug() << "Returning qvariant from value";
        qDebug() << "And its";
        qDebug() << m_backend;
        qDebug() << m_backend.data();
        qDebug() << m_backend.data()->devices();
        qDebug() << m_backend->devices();
        qDebug() << m_backend->devices().at(0);
        qDebug() << m_backend->devices().at(index.row());*/
        return QVariant::fromValue(m_backend->devices().at(index.row()));
    }
    return QVariant();
}

QObject *DeclarativeDevices::get(const int index) const
{
    return m_backend->devices().at(index);

    /*if (index < 0 || index >= rowCount()) {
        qDebug() << "Out of bounds";
        return QVariant();
    }*/
    /*qDebug() << "Everything good";
    QVariant device = data(createIndex(index, 0), DeviceRole);;*/
    //qDebug() << index << device << device.value<QObject *>()->property("vendor");

    //return QVariant::fromValue(QString("meow"));
    //return data(createIndex(index, 0), DeviceRole);
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

    emit queryChanged(query);
}

QHash<int, QByteArray> DeclarativeDevices::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[DeviceRole] = "device";
    return roles;
}

/*QObject *DeclarativeDevices::device(const QString &udi, const QString &_type)
{
    Solid::DeviceInterface::Type type = Solid::DeviceInterface::stringToType(_type);
    return Solid::Device(udi).asDeviceInterface(type);
}*/

} // namespace Solid
