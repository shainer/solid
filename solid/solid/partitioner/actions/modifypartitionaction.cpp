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
#include <solid/partitioner/actions/modifypartitionaction.h>

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

class ModifyPartitionAction::Private
{
public:    
    Private(const QStringList& fs)
        : isLabelChanged(false)
        , flags(fs)
    {}
    
    Private(const QString& l, const QStringList& fs)
        : isLabelChanged(true)
        , label(l)
        , flags(fs)
    {}
    
    ~Private()
    {}
    
    void setDescription()
    {
        QString descPart1 = "Setting label of %0 to \"%1\".";
        QString descPart2, desc;
        
        if (isLabelChanged) {
            desc += descPart1.arg(q->partition(), label);
        }
        
        if (isLabelChanged) {
            descPart2 = "Setting the following flags: %0";
            desc += descPart2.arg( flags.join(" ") );
        }
        else {
            descPart2 = "Setting the following flags for %0: %1";
            desc += descPart2.arg( q->partition(), flags.join(" ") );
        }

        q->setDescription(desc);
    }
    
    ModifyPartitionAction* q;
    bool isLabelChanged;
    QString label;
    QStringList flags;
};

ModifyPartitionAction::ModifyPartitionAction(const QString& partition,
                                             const QString& label,
                                             const QStringList& flags)
    : Action(partition)
    , d( new Private(label, flags) )
{
    d->q = this;
    d->setDescription();
}

ModifyPartitionAction::ModifyPartitionAction(const QString& partition,
                                             const QStringList& flags)
    : Action(partition)
    , d( new Private(flags) )
{
    d->q = this;
    d->setDescription();
}

ModifyPartitionAction::~ModifyPartitionAction()
{
    delete d;
}

Action::ActionType ModifyPartitionAction::actionType() const
{
    return ModifyPartition;
}

QString ModifyPartitionAction::label() const
{
    return d->label;
}

bool ModifyPartitionAction::isLabelChanged() const
{
    return d->isLabelChanged;
}

QStringList ModifyPartitionAction::flags() const
{
    return d->flags;
}

}
}
}
