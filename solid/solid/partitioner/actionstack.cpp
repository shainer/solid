/*
    Copyright 2012 Lisa Vitolo <shainer@chakra-project.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) version 3, or any
    later version accepted by the membership of KDE e.V. (or its
    successor approved by the membership of KDE e.V.), which shall
    act as a proxy defined in Section 6 of version 3 of the license.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library. If not, see <http://www.gnu.org/licenses/>.
*/
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
    
    d->actions.push_back(op);
}

QList< Action* > ActionStack::undo()
{
    Action* first = NULL;
    
    if (!d->actions.isEmpty()) {
        first = d->actions.takeLast();
        d->undoneActions.push_back(first);
    }
    
    QList< Action* > undone = d->actions;
    d->actions.clear();
    
    return undone;
}

Action* ActionStack::redo()
{
    Action* firstUndone = NULL;
    
    if (!d->undoneActions.isEmpty()) {
        firstUndone = d->undoneActions.takeLast();
    }
    
    return firstUndone;
}

void ActionStack::removeAction(Action* action)
{
    for (QList< Action* >::iterator it = d->actions.begin(); it != d->actions.end(); it++) {
        if ((*it)->description() == action->description()) {
            d->actions.erase(it);
        }
    }
    
    delete action;
}

void ActionStack::removeActionsOfDisk(const QString& diskName)
{    
    for (QList< Action* >::iterator it = d->actions.begin(); it != d->actions.end(); it++) {
        if ((*it)->ownerDisk()->name() == diskName) {
            d->actions.erase(it);
        }
    }
    
    for (QList< Action* >::iterator it = d->undoneActions.begin(); it != d->undoneActions.end(); it++) {
        if ((*it)->ownerDisk()->name() == diskName) {
            d->undoneActions.erase(it);
        }
    }
}

bool ActionStack::contains(Action* a) const
{
    return d->actions.contains(a);
}

int ActionStack::size() const
{
    return d->actions.size();
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