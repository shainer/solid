#ifndef SOLID_PARTITIONER_DEVICES_DEVICEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_DEVICEMODIFIED_H

#include <solid/deviceinterface.h>
#include <QtCore/QSharedDataPointer>

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {
            class DeviceModifiedPrivate : public QSharedData
            {
            public:
                DeviceModifiedPrivate(DeviceInterface* i)
                    : iface(i)
                    , existent(i->isValid())
                {}
                
                DeviceModifiedPrivate()
                    : iface(0)
                    , existent(false)
                {}
                
                DeviceInterface* iface;
                QString udi;
                QString parentUdi;
                bool existent;
            };
            
            class DeviceModified
            {
            public:
                enum DeviceModifiedType
                {
                    StorageDriveType,
                    StorageVolumeType,
                    FreeSpaceType
                };
                
                explicit DeviceModified(DeviceInterface *);
                DeviceModified();
                DeviceModified(const DeviceModified &);
                virtual ~DeviceModified();
                
                virtual DeviceModifiedType deviceType() const = 0;
                bool existent() const;
                QString udi() const;
                QString parentUdi() const;
                
                void setUdi(const QString &);
                void setParentUdi(const QString &);
                void setExistent(bool);
                
            private:
                QSharedDataPointer<DeviceModifiedPrivate> d;
            };
            
        }
    }
}

#endif
