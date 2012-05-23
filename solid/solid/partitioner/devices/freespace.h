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
                FreeSpacePrivate(qulonglong s, qulonglong o, const QString& p)
                    : QSharedData()
                    , size(s)
                    , offset(o)
                    , parentUdi(p)
                {}
                
                FreeSpacePrivate(const FreeSpacePrivate &other)
                    : QSharedData()
                    , size(other.size)
                    , offset(other.offset)
                    , parentUdi(other.parentUdi)
                {}
                
                ~FreeSpacePrivate()
                {}
                
                qulonglong size;
                qulonglong offset;
                QString parentUdi;
            };
            
            class FreeSpace : public DeviceModified
            {
            public:
                explicit FreeSpace(qulonglong, qulonglong, const QString&);
                FreeSpace(const FreeSpace &);
                virtual ~FreeSpace();
                
                virtual DeviceModifiedType deviceType() const;
                qulonglong size() const;
                qulonglong offset() const;
                QString parentUdi() const;
                
                void setSize(qulonglong);
                void setOffset(qulonglong);
                
            private:
                QSharedDataPointer<FreeSpacePrivate> d;
            };
            
        }
    }
}

#endif