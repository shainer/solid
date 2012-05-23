#ifndef SOLID_PARTITIONER_ACTIONS_ACTION_H
#define SOLID_PARTITIONER_ACTIONS_ACTION_H

#include <solid/solid_export.h>
#include <QtCore/QObject>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            class SOLID_EXPORT Action
            {                
            public:
                enum ActionType {
                    CreatePartition,
                    RemovePartition,
                    FormatPartition
                };
                
                explicit Action() {}
                virtual ~Action() {}
                
                virtual Action::ActionType actionType() const = 0;
            };
        }
    }
}

#endif
