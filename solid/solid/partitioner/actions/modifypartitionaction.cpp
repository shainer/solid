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
    
    bool isLabelChanged;
    
    QString label;
    QStringList flags;
};

ModifyPartitionAction::ModifyPartitionAction(const QString& partition,
                                             const QString& label,
                                             const QStringList& flags)
    : PartitionAction(partition)
    , d( new Private(label, flags) )
{}

ModifyPartitionAction::ModifyPartitionAction(const QString& partition,
                                             const QStringList& flags)
    : PartitionAction(partition)
    , d( new Private(flags) )
{}


ModifyPartitionAction::~ModifyPartitionAction()
{
    delete d;
}

Action::ActionType ModifyPartitionAction::actionType() const
{
    return ModifyPartition;
}

QString ModifyPartitionAction::description() const
{
    QString desc1 = " Setting the following flags for %0: %1.";
    QString desc3 = "Setting label of %0 to %1.";
    QString desc;
    
    if (d->isLabelChanged) {
        desc += desc3.arg(partition(), d->label);
    }
    if (!d->flags.isEmpty()) {
        desc += desc1.arg(partition(), d->flags.join(" "));
    }

    return desc;
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
