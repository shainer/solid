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
#include <solid/partitioner/actions/modifyfilesystemaction.h>

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

class ModifyFilesystemAction::Private
{
public:
    Private(const QString& l)
        : fsLabel(l)
    {}
    
    QString fsLabel;
};

ModifyFilesystemAction::ModifyFilesystemAction(const QString& partition, const QString& fsLabel)
    : Action(partition)
    , d( new Private(fsLabel) )
{
    QString desc( "Setting a new label for the filesystem of %0: %1" );
    desc = desc.arg(partition, fsLabel);
    
    setDescription(desc);
}

ModifyFilesystemAction::~ModifyFilesystemAction()
{
    delete d;
}

Action::ActionType ModifyFilesystemAction::actionType() const
{
    return ModifyFilesystem;
}

QString ModifyFilesystemAction::fsLabel() const
{
    return d->fsLabel;
}

}
}
}
