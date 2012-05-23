#ifndef SOLID_ACTION_H
#define SOLID_ACTION_H

#include <solid/solid_export.h>
#include <QtCore/QObject>

namespace Solid
{
    namespace Partitioner
    {
        namespace Actions
        {
            class SOLID_EXPORT Action : QObject
            {
                Q_OBJECT
                Q_DISABLE_COPY(Action)
                
                Q_ENUMS(ActionType)
                
            public:
                enum ActionType {
                    CreatePartition = 0,
                    RemovePartition = 1,
                    FormatPartition = 2
                };
                
                explicit Action() {}
                virtual ~Action() {}
                
                virtual ActionType actionType() const =0;
            };            
        }
    }
}

#endif
