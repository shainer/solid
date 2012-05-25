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
    Private(const QString& p, const QString& l)
        : isLabelChanged(true)
        , isFlagChanged(false)
        , partition(p)
        , label(l)
    {}
    
    Private(const QString& p, const QStringList& f)
        : isLabelChanged(false)
        , isFlagChanged(true)
        , partition(p)
        , flags(f)
    {}
    
    Private(const QString& p, const QString& l, const QStringList& f)
        : isLabelChanged(true)
        , isFlagChanged(true)
        , partition(p)
        , label(l)
        , flags(f)
    {}
    
    ~Private()
    {}
    
    bool isLabelChanged;
    bool isFlagChanged;
    
    QString partition;
    QString label;
    QStringList flags;
};

ModifyPartitionAction::ModifyPartitionAction(const QString& partition, const QString& label, const QStringList& flags)
    : d( new Private(partition, label, flags) )
{}

ModifyPartitionAction::ModifyPartitionAction(const QString& partition, const QString& label)
    : d( new Private(partition, label) )
{}

ModifyPartitionAction::ModifyPartitionAction(const QString& partition, const QStringList& flags)
    : d( new Private(partition, flags) )
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
    QString desc1 = "Setting flags of %0 = %1";
    QString desc2 = "Setting label of %0 to %1";
    
    if (d->isLabelChanged && !d->isFlagChanged) {
        desc2 = desc2.arg(d->partition, d->label);
        return QObject::tr( desc2.toUtf8().data() );
    }
    if (!d->isLabelChanged && d->isFlagChanged) {
        desc1 = desc1.arg(d->partition, d->flags.join(", "));
        return QObject::tr( desc1.toUtf8().data() );
    }
    
    return desc1 + " and " + desc2;
}

QString ModifyPartitionAction::partition() const
{
    return d->partition;
}

QString ModifyPartitionAction::label() const
{
    return d->label;
}

bool ModifyPartitionAction::bootable() const
{
    return d->flags.contains("boot");
}

bool ModifyPartitionAction::required() const
{
    return d->flags.contains("required");
}

bool ModifyPartitionAction::isLabelChanged() const
{
    return d->isLabelChanged;
}

bool ModifyPartitionAction::isFlagChanged() const
{
    return d->isFlagChanged;
}


}
}
}