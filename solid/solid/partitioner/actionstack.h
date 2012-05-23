#ifndef SOLID_PARTITIONER_ACTIONSTACK_H
#define SOLID_PARTITIONER_ACTIONSTACK_H

#include <QtCore/QList>
#include <solid/partitioner/actions/action.h>

namespace Solid
{
    namespace Partitioner
    {
        using namespace Actions;
    
        class ActionStack
        {    
        public:
            ActionStack();
            virtual ~ActionStack();
            
            void push(Action *op);
            QList<Action *> undo();
            QList<Action *> redo();
            void clear();

            bool empty() const;
            bool undoEmpty() const;
            
            QList<Action *> list() const;
            QList<Action *> undoList() const;
            
        private:
            class Private;
            Private* d;
        };
    
    }

}

#endif