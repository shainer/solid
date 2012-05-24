/*
    Copyright 2010 Michael Zanetti <mzanetti@kde.org>
    Copyright 2010-2011 Lukas Tinkl <ltinkl@redhat.com>

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

#ifndef UDISKSDEVICE_H
#define UDISKSDEVICE_H

#include <ifaces/device.h>
#include <solid/deviceinterface.h>
#include <solid/solidnamespace.h>

#include <QtDBus/QDBusInterface>
#include <QtCore/QSet>
#include <QtCore/QStringList>
#include <kauthactionreply.h>

QString formatByteSize(double);

namespace Solid
{
namespace Backends
{
namespace UDisks
{

using namespace KAuth;
    
class UDisksDevice : public Solid::Ifaces::Device
{
    Q_OBJECT

public:
    typedef enum {
        ReadCache,
        BypassCache,
    } CacheBehaviour;
    
    UDisksDevice(const QString &udi);
    virtual ~UDisksDevice();

    virtual QObject* createDeviceInterface(const Solid::DeviceInterface::Type& type);
    virtual bool queryDeviceInterface(const Solid::DeviceInterface::Type& type) const;
    virtual QString description() const;
    virtual QStringList emblems() const;
    virtual QString icon() const;
    virtual QString product() const;
    virtual QString vendor() const;
    virtual QString udi() const;
    virtual QString parentUdi() const;

    /*
     * If behaviour is set to "BypassCache", the correspondent property is never read from the cache,
     * but taken directly from UDisks each time. This is needed for the properties concerning accessibility:
     * for those, the Changed signal always arrives too late, thus the cache isn't updated when we go reading the
     * new accessibility status of a device.
     */
    QVariant prop(const QString &key, CacheBehaviour behaviour = ReadCache) const;
    bool propertyExists(const QString &key) const;
    QMap<QString, QVariant> allProperties() const;

    bool isDeviceBlacklisted() const;

    QString errorToString(const QString & error) const;
    Solid::ErrorType errorToSolidError(const QString & error) const;
    
    
    bool format(const QString& filesystemName,
                        const QStringList& filesystemFlags = QStringList());
    bool deletePartition(const QStringList& options = QStringList());
    
    bool modifyPartition(const QString& type,
                         const QString& label,
                         const QStringList& flags = QStringList());
    bool setFilesystemLabel(const QString& label);
    bool createTable(const QString& scheme, const QStringList& options = QStringList());
    
    QDBusObjectPath createPartition(qulonglong offset,
                                    qulonglong size,
                                    const QString& type,
                                    const QString& label = QString(),
                                    const QStringList& flags = QStringList(),
                                    const QStringList& options = QStringList(),
                                    const QString& filesystem = QString(),
                                    const QStringList& filesystemFlags = QStringList());
    bool safelyResizePartition(qulonglong oldSize,
                               qulonglong newSize,
                               const QString& filesystem);
                               
    
    QString latestError() const;
    
Q_SIGNALS:
    void changed();

private Q_SLOTS:
    void slotChanged();
    void resizeDone(ActionReply);

private:
    QString storageDescription() const;
    QString volumeDescription() const;
    mutable QDBusInterface *m_device;
    QString m_udi;
    mutable QVariantMap m_cache;
    QString errorDescription;

    void checkCache(const QString &key, CacheBehaviour behaviour) const;
};

}
}
}

#endif // UDISKSDEVICE_H
