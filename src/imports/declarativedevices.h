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

#ifndef SOLID_DECALARATIVE_DEVICES_H
#define SOLID_DECALARATIVE_DEVICES_H

#include <QObject>
#include <solid/predicate.h>
#include <solid/deviceinterface.h>

#include <QAbstractListModel>
#include <QList>
#include <QSharedPointer>

namespace Solid
{

class DeviceNotifier;
class DevicesQueryPrivate;

/**
 * A class that gives access to devices known to the
 * Solid system.
 *
 * It is implemented as a model and allows for easy
 * display of devices in a ListView, for example
 * in the Battery Monitor. Itallows to only expose
 * devices matching a specified query (formatted for
 * Solid::Predicate).
 *
 * It is intended to be used from QML like this:
 *
 * <code>
 *    Solid.Devices {
 *        id: allDevices
 *    }
 *
 *    Solid.Devices {
 *        id: networkShares
 *        query: "IS NetworkShare"
 *    }
 *
 *    Solid.Devices {
 *        id: mice
 *        query: "PointingDevice.type == 'Mouse'"
 *    }
 *
 *    Text {
 *        text: "Total number of devices: " + allDevices.count
 *    }
 *
 *    Text {
 *        text: "NFS url: " + networkShares.get(0).url
 *    }
 * </code>
 */
class DeclarativeDevices: public QAbstractListModel
{
    Q_OBJECT

    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY rowCountChanged)
    Q_PROPERTY(bool empty READ isEmpty NOTIFY emptyChanged)

public:
    enum ModelRoles {
        DeviceRole = Qt::UserRole + 1
    };

    explicit DeclarativeDevices(QAbstractListModel *parent = Q_NULLPTR);
    ~DeclarativeDevices();

Q_SIGNALS:
    /**
     * Emitted when a new device matching the specified
     * query arrives
     * @param udi UDI of the new device
     */
    void deviceAdded(const QObject *device) const;

    /**
     * Emitted when a device matching the specified
     * query disappears
     * @param udi UDI of the device
     */
    void deviceRemoved(const QString &udi) const;

    /**
     * Emitted when the number of devices that
     * match the specified query has changed
     * @param count new device count
     */
    void rowCountChanged(int count) const;

    /**
     * Emitted when the query has changed
     * @param query new query
     */
    void queryChanged(const QString &query) const;

    /**
     * Emitted when the empty property changes
     * @param empty is the device list empty
     */
    void emptyChanged(bool empty) const;

public:
    /**
     * Retrieves the number of the devices that
     * match the specified query
     * @return device count
     */
    int rowCount(const QModelIndex & parent = QModelIndex()) const;

    /**
     * Retrieves whether there are devices matching
     * the specified query
     * @return true if there are no matching devices
     */
    bool isEmpty() const;

    /**
     * Retrieves the list of UDIs of the devices that
     * match the specified query
     */
    QList<QObject *> devices() const;

    /**
     * Query to check the devices against. It needs
     * to be formatted for Solid::Predicate.
     * @see Solid::Predicate
     */
    QString query() const;

    /**
     * Sets the query to filter the devices.
     * @param query new query
     */
    void setQuery(const QString &query);

public Q_SLOTS:
    /**
     * Retrieves a device from the model
     *
     * @param index the index of the item
     * @return the device at the given index
     */
    QObject *get(const int index) const;

private Q_SLOTS:
    /**
     * A device was added
     *
     * @param device the pointer to the device
     */
    void addDevice(const QObject *device);
    /**
     * A device was removed
     *
     * This is to eventually tell interested parties
     * about the UDI of the device removed
     * @param udi udi of the device
     */
    void removeDevice(const QString &udi);

    /**
     * A device object was destroyed
     *
     * This is used internally to remove the affected
     * index from the model
     */
    void removeDeviceFromModel(const int index);

    /**
     * Initializes the backend object
     */
    void initialize() const;

    /**
     * Frees up the backend and sends the appropriate events
     */
    void reset();

protected:
    QHash<int, QByteArray> roleNames() const;

private:
    Q_DISABLE_COPY(DeclarativeDevices)

    QVariant data(const QModelIndex &index, int role) const;

    QString m_query;

    mutable QSharedPointer<DevicesQueryPrivate> m_backend;
};

} // namespace Solid

#endif

