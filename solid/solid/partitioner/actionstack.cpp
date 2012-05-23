#include <solid/partitioner/actionstack.h>

namespace Solid
{
namespace Partitioner
{

using namespace Actions;
    
class ActionStack::Private
{
public:
    Private()
    {}
    
    ~Private()
    {
        qDeleteAll(actions);
        qDeleteAll(undoneActions);
    }
    
    QList<Action *> actions;
    QList<Action *> undoneActions;
};
    
ActionStack::ActionStack()
    : d( new Private )
{}

ActionStack::~ActionStack()
{
    delete d;
}

void ActionStack::push(Action* op)
{
    qDeleteAll(d->undoneActions);
    d->undoneActions.clear();
    
    d->actions.push_front(op);
}

QList< Action* > ActionStack::undo()
{
    Action* first = NULL;
    
    if (!d->actions.isEmpty()) {
        first = d->actions.takeFirst();
        d->undoneActions.push_front(first);
    }
    
    return d->actions;
}

QList< Action* > ActionStack::redo()
{
    Action* firstUndone = NULL;
    
    if (!d->undoneActions.isEmpty()) {
        firstUndone = d->undoneActions.takeFirst();
        d->actions.push_front(firstUndone);
    }
    
    return d->actions;
}

bool ActionStack::empty() const
{
    return d->actions.isEmpty();
}

bool ActionStack::undoEmpty() const
{
    return d->undoneActions.isEmpty();
}

QList< Action* > ActionStack::list() const
{
    return d->actions;
}

QList< Action* > ActionStack::undoList() const
{
    return d->undoneActions;
}

void ActionStack::clear()
{
    qDeleteAll(d->actions);
    qDeleteAll(d->undoneActions);
    
    d->actions.clear();
    d->undoneActions.clear();
}

}
}