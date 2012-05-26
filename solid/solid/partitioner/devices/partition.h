#ifndef SOLID_PARTITIONER_DEVICES_PARTITION_H
#define SOLID_PARTITIONER_DEVICES_PARTITION_H

#include "devicemodified.h"
#include <solid/partitioner/actions/createpartitionaction.h>
#include <solid/partitioner/utils/partitioner_enums.h>
#include <solid/partitioner/utils/filesystem.h>
#include <solid/storagevolume.h>
#include <unistd.h>

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
                
                Utils::Filesystem filesystem() const;
                
                QString label() const;
                QString uuid() const;
                qulonglong size() const;
                virtual qulonglong offset() const;
                qulonglong rightBoundary() const;
                Utils::PartitionType partitionType() const;
                bool isFlagSet(const QString &) const;
                
                void setIgnored(bool);
                void setUsage(StorageVolume::UsageType);
                void setFilesystem(const Utils::Filesystem &);
                void setLabel(const QString &);
                void setSize(qulonglong);
                void setOffset(qulonglong);
                void setPartitionType(Utils::PartitionType);
                void setFlag(const QString &);
                void unsetFlag(const QString &);
                void setFlags(const QStringList &);

            private:
                class Private;
                Private* d;
            };
            
        }
    }
}

#endif
