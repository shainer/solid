#ifndef SOLID_PARTITIONER_DEVICES_STORAGEDRIVEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_STORAGEDRIVEMODIFIED_H
#include "devicemodified.h"
#include <solid/storagedrive.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {            
            class SOLID_EXPORT Disk : public DeviceModified
            {
            public:
                explicit Disk(StorageDrive *);
                virtual ~Disk();
                
                DeviceModifiedType deviceType() const;
                
                qulonglong size() const;
                void setSize(qulonglong);
                
            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif