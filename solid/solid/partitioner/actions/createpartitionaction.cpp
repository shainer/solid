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
#include "createpartitionaction.h"
#include <partitioner/volumemanager.h>
#include <partitioner/utils/utils.h>

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

using namespace Utils;
    
class CreatePartitionAction::Private
{
public:
    Private(const QString& d, qulonglong o, qulonglong s, bool e, Utils::Filesystem fs, const QString& l, const QStringList& f)
        : disk(d)
        , offset(o)
        , size(s)
        , extended(e)
        , logical(false)
        , filesystem(fs)
        , label(l)
        , flags(f)
    {
        QMap<QString, VolumeTree> trees = VolumeManager::instance()->allDiskTrees();
        VolumeTree diskLayout = trees[disk];
        
        if (trees.contains(disk)) {
            DeviceModified* extended = diskLayout.extendedPartition();
            
            if (extended && (extended->offset() <= offset && extended->rightBoundary() >= offset + size)) {
                logical = true;
            }
        }
    }
    
    ~Private()
    {}
    
    QString disk;
    qulonglong offset;
    qulonglong size;
    bool extended;
    bool logical;
    Utils::Filesystem filesystem;
    QString label;
    QStringList flags;
    
    QString newPartitionName;
};

CreatePartitionAction::CreatePartitionAction(const QString& disk,
                                             qulonglong offset,
                                             qulonglong size,
                                             bool extended,
                                             const Utils::Filesystem& fs,
                                             const QString& label,
                                             const QStringList& flags
                                            )
    : d( new Private(disk, offset, size, extended, fs, label, flags) )
{}

CreatePartitionAction::CreatePartitionAction(const QString& disk,
                                             qulonglong offset,
                                             qulonglong size,
                                             const Utils::Filesystem& fs,
                                             const QString& label,
                                             const QStringList& flags)
    : d( new Private(disk, offset, size, false, fs, label, flags) )
{}

CreatePartitionAction::~CreatePartitionAction()
{
    delete d;
}

Action::ActionType CreatePartitionAction::actionType() const
{
    return Action::CreatePartition;
}

QString CreatePartitionAction::description() const
{
    QString desc( "Creating a new partition with size %0 and \"%1\" filesystem on %2" );
    desc = desc.arg(QString::number(d->size), d->filesystem.name(), d->disk);
    
    return QObject::tr(desc.toUtf8().data());
}


QString CreatePartitionAction::disk() const
{
    return d->disk;
}

qulonglong CreatePartitionAction::offset() const
{
    return d->offset;
}

qulonglong CreatePartitionAction::size() const
{
    return d->size;
}

PartitionType CreatePartitionAction::partitionType() const
{
    if (d->extended) {
        return ExtendedPartition;
    }
    
    if (d->logical) {
        return LogicalPartition;
    }
    
    return PrimaryPartition;
}

Filesystem CreatePartitionAction::filesystem() const
{
    return d->filesystem;
}

QString CreatePartitionAction::label() const
{
    return d->label;
}

QStringList CreatePartitionAction::flags() const
{
    return d->flags;
}

QString CreatePartitionAction::partitionName() const
{
    return d->newPartitionName;
}

void CreatePartitionAction::setPartitionName(const QString& name)
{
    d->newPartitionName = name;
}
    
}
}
}
