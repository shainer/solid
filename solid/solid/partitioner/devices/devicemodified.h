#ifndef SOLID_PARTITIONER_DEVICES_DEVICEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_DEVICEMODIFIED_H

#include <solid/deviceinterface.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {            
            class SOLID_EXPORT DeviceModified
            {
            public:
                enum DeviceModifiedType
                {
                    DiskDevice,
                    PartitionDevice,
                    FreeSpaceDevice
                };
                
                explicit DeviceModified(DeviceInterface *);
                DeviceModified();
                virtual ~DeviceModified();
                
                virtual DeviceModifiedType deviceType() const = 0;
                virtual qulonglong offset() const;
                virtual qulonglong size() const = 0;
                
                bool existent() const;
                virtual QString name() const;
                virtual QString parentName() const;
                
                void setName(const QString &);
                void setParentName(const QString &);
                void setExistent(bool);
                
                bool operator==(const DeviceModified &) const;
                
            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
