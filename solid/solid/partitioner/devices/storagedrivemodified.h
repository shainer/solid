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
            
            class StorageDriveModifiedPrivate;
            
            class SOLID_EXPORT StorageDriveModified : public DeviceModified
            {
            public:
                explicit StorageDriveModified(StorageDrive *);
                StorageDriveModified(const StorageDriveModified& );
                virtual ~StorageDriveModified();
                
                DeviceModifiedType deviceType() const;
                
                qulonglong size() const;
                void setSize(qulonglong);
                
            private:
                QSharedDataPointer<StorageDriveModifiedPrivate> d;
            };
            
        }
    }
}

#endif