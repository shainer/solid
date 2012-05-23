#ifndef SOLID_PARTITIONER_DEVICES_FREESPACE_H
#define SOLID_PARTITIONER_DEVICES_FREESPACE_H
#include "devicemodified.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Devices
        {
            class FreeSpacePrivate : public QSharedData
            {
            public:
                FreeSpacePrivate(qulonglong o, qulonglong s)
                    : QSharedData()
                    , offset(o)
                    , size(s)
                {}
                
                FreeSpacePrivate(const FreeSpacePrivate &other)
                    : QSharedData()
                    , offset(other.offset)
                    , size(other.size)
                {}
                
                ~FreeSpacePrivate()
                {}
                
                qulonglong offset;
                qulonglong size;
            };
            
            class SOLID_EXPORT FreeSpace : public DeviceModified
            {
            public:
                explicit FreeSpace(qulonglong, qulonglong, const QString&);
                FreeSpace(const FreeSpace &);
                virtual ~FreeSpace();
                
                virtual DeviceModifiedType deviceType() const;
                qulonglong size() const;
                qulonglong offset() const;
                
                void setSize(qulonglong);
                void setOffset(qulonglong);
                
            private:
                QSharedDataPointer<FreeSpacePrivate> d;
            };
            
        }
    }
}

#endif