#ifndef SOLID_PARTITIONER_DEVICES_STORAGEVOLUMEMODIFIED_H
#define SOLID_PARTITIONER_DEVICES_STORAGEVOLUMEMODIFIED_H

#include "devicemodified.h"
#include <solid/storagevolume.h>

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {
            class StorageVolumeModifiedPrivate;
            
            class SOLID_EXPORT StorageVolumeModified : public DeviceModified
            {
            public:
                enum PartitionType
                {
                    None,
                    Primary,
                    Logical,
                    Extended
                };
                
                explicit StorageVolumeModified(StorageVolume* );
                StorageVolumeModified(const StorageVolumeModified &);
                virtual ~StorageVolumeModified();
                
                DeviceModified::DeviceModifiedType deviceType() const;
                
                bool ignored() const;
                StorageVolume::UsageType usage() const;
                QString filesystem() const;
                QString label() const;
                QString uuid() const;
                qulonglong size() const;
                qulonglong offset() const;
                qulonglong rightBoundary() const;
                PartitionType partitionType() const;
                
                void setIgnored(bool);
                void setUsage(StorageVolume::UsageType);
                void setFilesystem(const QString &);
                void setLabel(const QString &);
                void setSize(qulonglong);
                void setOffset(qulonglong);
                void setPartitionType(PartitionType);
                
            private:
                QSharedDataPointer<StorageVolumeModifiedPrivate> d;
            };
            
        }
    }
}

#endif