#ifndef SOLID_PARTITIONER_ACTIONS_RESIZEPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_RESIZEPARTITIONACTION_H
#include "action.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            class SOLID_EXPORT ResizePartitionAction : public Action
            {
            public:
                explicit ResizePartitionAction(const QString& partition, qlonglong newOffset, qlonglong newSize = -1);
                explicit ResizePartitionAction(const QString& partition, qlonglong newSize);
                virtual ~ResizePartitionAction();
                
                ActionType actionType() const;
                QString partition() const;
                qlonglong newSize() const;
                qlonglong newOffset() const;
                
            private:
                QString m_partition;
                qlonglong m_newOffset;
                qlonglong m_newSize;
            };
        }
    }
}

#endif