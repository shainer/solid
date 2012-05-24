#ifndef SOLID_PARTITIONER_ACTIONS_REMOVEPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_REMOVEPARTITIONACTION_H
#include "action.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            class SOLID_EXPORT RemovePartitionAction : public Action
            {
            public:
                explicit RemovePartitionAction(const QString &);
                virtual ~RemovePartitionAction();
                
                ActionType actionType() const;
                QString partition() const;
                
            private:
                QString m_partition;
            };
        }
    }
}

#endif