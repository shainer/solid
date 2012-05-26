#ifndef SOLID_PARTITIONER_DEVICES_STORAGEDRIVEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_STORAGEDRIVEMODIFIED_H

#include "devicemodified.h"
#include <solid/storagedrive.h>
#include <solid/partitioner/utils/partitiontableutils.h>
#include <solid/partitioner/utils/partitioner_enums.h>

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
                qulonglong offset() const;
                qulonglong rightBoundary() const;
                Utils::PTableType partitionTableScheme() const;
                
                void setSize(qulonglong);
                void setOffset(qulonglong);
                void setPartitionTableScheme(Utils::PTableType);

            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
