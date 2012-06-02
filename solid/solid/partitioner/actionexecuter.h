#ifndef SOLID_PARTITIONER_ACTIONEXECUTER_H
#define SOLID_PARTITIONER_ACTIONEXECUTER_H

#include <solid/solid_export.h>
#include <QtCore/QObject>
#include "actions/action.h"
#include "volumetree.h"

namespace Solid
{
    namespace Partitioner
    {
    
        class ActionExecuter : public QObject
        {
            Q_OBJECT
            
        public:
            ActionExecuter();
            virtual ~ActionExecuter();
            
            bool execute();
            
        private:
            QList< Actions::Action* > m_actions;
            QMap<QString, VolumeTree> m_disks;
        };
    
    }
    
}

#endif