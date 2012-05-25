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
                explicit ResizePartitionAction(const QString &, qulonglong, qulonglong);
                virtual ~ResizePartitionAction();
                
                ActionType actionType() const;
                QString partition() const;
                qulonglong newSize() const;
                qulonglong newOffset() const;
                
            private:
                QString m_partition;
                qulonglong m_newOffset;
                qulonglong m_newSize;
            };
        }
    }
}

#endif