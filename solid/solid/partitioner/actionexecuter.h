#ifndef SOLID_PARTITIONER_ACTIONEXECUTER_H
#define SOLID_PARTITIONER_ACTIONEXECUTER_H

#include <solid/solid_export.h>
#include <QtCore/QObject>
#include "actions/action.h"

namespace Solid
{
    namespace Partitioner
    {
    
        class ActionExecuter : public QObject
        {
            Q_OBJECT
            
        public:
            ActionExecuter(const QList<Actions::Action *>& );
            virtual ~ActionExecuter();
            
            bool valid() const;
                    
        private:
            QList<Actions::Action *> actions;
            bool m_valid;
        };
    
    }
    
}

#endif