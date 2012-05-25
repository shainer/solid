#ifndef SOLID_PARTITIONER_DEVICES_PARTITION_H
#define SOLID_PARTITIONER_DEVICES_PARTITION_H

#include "devicemodified.h"
#include <solid/partitioner/actions/createpartitionaction.h>
#include <solid/partitioner/partitioner_enums.h>
#include <solid/storagevolume.h>

namespace Solid
{
    namespace Partitioner
    {        
        namespace Devices
        {            
            class SOLID_EXPORT Partition : public DeviceModified
            {
            public:                
                explicit Partition(StorageVolume *);
                Partition(Actions::CreatePartitionAction *);
                virtual ~Partition();
                
                DeviceModified::DeviceModifiedType deviceType() const;
                
                bool ignored() const;
                StorageVolume::UsageType usage() const;
                QString filesystem() const;
                QString label() const;
                QString uuid() const;
                qulonglong size() const;
                virtual qulonglong offset() const;
                qulonglong rightBoundary() const;
                PartitionType partitionType() const;
                bool bootable() const;
                bool required() const;
                
                void setIgnored(bool);
                void setUsage(StorageVolume::UsageType);
                void setFilesystem(const QString &);
                void setLabel(const QString &);
                void setSize(qulonglong);
                void setOffset(qulonglong);
                void setPartitionType(PartitionType);
                void setBootable(bool);
                void setRequired(bool);
                                
            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif