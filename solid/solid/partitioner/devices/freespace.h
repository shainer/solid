#ifndef SOLID_PARTITIONER_DEVICES_FREESPACE_H
#define SOLID_PARTITIONER_DEVICES_FREESPACE_H

#include "devicemodified.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {            
            class SOLID_EXPORT FreeSpace : public DeviceModified
            {
            public:
                explicit FreeSpace(qulonglong, qulonglong, const QString&);
                virtual ~FreeSpace();
                
                virtual DeviceModifiedType deviceType() const;
                qulonglong size() const;
                qulonglong offset() const;
                
                void setSize(qulonglong);
                void setOffset(qulonglong);
                
            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif