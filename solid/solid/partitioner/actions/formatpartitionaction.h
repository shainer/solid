#ifndef SOLID_PARTITIONER_ACTIONS_FORMATPARTITIONACTION_H
#define SOLID_PARTITIONER_ACTIONS_FORMATPARTITIONACTION_H
#include "action.h"

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            class SOLID_EXPORT FormatPartitionAction : public Action
            {
            public:
                explicit FormatPartitionAction(const QString &, const QString &);
                virtual ~FormatPartitionAction();
                
                virtual Action::ActionType actionType() const;
                
                QString partition() const;
                QString filesystem() const;
                
            private:
                QString m_partition;
                QString m_filesystem;
            };
        }
    }
}

#endif