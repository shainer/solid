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
#include <solid/partitioner/actionexecuter.h>
#include <solid/partitioner/volumemanager.h>
#include <solid/partitioner/actions/formatpartitionaction.h>
#include <solid/partitioner/actions/createpartitiontableaction.h>
#include <solid/partitioner/actions/removepartitiontableaction.h>
#include <solid/partitioner/actions/removepartitionaction.h>
#include <solid/partitioner/actions/modifypartitionaction.h>
#include <solid/partitioner/actions/resizepartitionaction.h>
#include <solid/partitioner/utils/partitiontableutils.h>
#include <ifaces/devicemanager.h>
#include <backends/udisks/udisksmanager.h>
#include <kglobal.h>

#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>

namespace Solid
{
namespace Partitioner
{
    
using namespace Actions;
using namespace Utils;
using namespace Backends::UDisks;

class ActionExecuter::Private
{
public:
    Private(const QList< Action* >& a, const VolumeTreeMap& m)
        : actions(a)
        , map(m)
        , valid(true)
    {
        partitionTypes << Action::RemovePartition
                       << Action::ResizePartition
                       << Action::FormatPartition
                       << Action::ModifyPartition;
    }
    
    ~Private()
    {}
    
    void translateFutureNames(QList< Action* >::iterator, const QString &);
        
    QList< Action::ActionType > partitionTypes;
    QList< Action* > actions;
    VolumeTreeMap map;
    bool valid;
    Utils::PartitioningError error;
};
    
ActionExecuter::ActionExecuter(const QList< Action* >& actions, const VolumeTreeMap& map)
    : QObject()
    , d( new Private(actions, map) )
{}

ActionExecuter::~ActionExecuter()
{
    delete d;
}

bool ActionExecuter::isValid() const
{
    return d->valid;
}

Utils::PartitioningError ActionExecuter::error() const
{
    return d->error;
}

bool ActionExecuter::execute()
{
    if (!d->valid) {
        return false;
    }
    
    int actionCount = 0;
    
    for (QList< Action* >::iterator it = d->actions.begin(); it < d->actions.end(); it++) {
        Action* action = (*it);
        UDisksDevice* device;
        bool success = true;
        
        switch (action->actionType())
        {            
            case Action::FormatPartition: {
                FormatPartitionAction* fpa = dynamic_cast< FormatPartitionAction* >(action);
                
                device = new UDisksDevice( fpa->partition() );
                Utils::Filesystem fs = fpa->filesystem();
                
                success = device->format( fs.name(), fs.flags() );
                break;
            }
            
            case Action::CreatePartition: {
                CreatePartitionAction* cpa = dynamic_cast< CreatePartitionAction* >(action);
                
                device = new UDisksDevice( cpa->disk() );
                Disk* disk = dynamic_cast< Disk* >( d->map.searchDevice(cpa->disk()) );
                
                Filesystem fs = cpa->filesystem();
                QString type = (cpa->partitionType() == ExtendedPartition) ? "extended" : fs.name();
                QString typeUDisks = Utils::PartitionTableUtils::instance()->typeString(disk->partitionTableScheme(), type);
                
                QDBusObjectPath newPartition = device->createPartition(cpa->offset(), cpa->size(), typeUDisks, cpa->label(),
                                                                       cpa->flags(), QStringList(),
                                                                       fs.name(), fs.flags());
                if (newPartition.path().isEmpty()) {
                    success = false;
                }
                else {
                    d->translateFutureNames(it, newPartition.path());
                }
                
                break;
            }
            
            case Action::RemovePartition: {
                RemovePartitionAction* rpa = dynamic_cast< RemovePartitionAction* >(action);
                
                device = new UDisksDevice( rpa->partition() );
                success = device->deletePartition();
                
                break;
            }
            
            case Action::ModifyPartition: {
                ModifyPartitionAction* mpa = dynamic_cast< ModifyPartitionAction* >(action);
                
                device = new UDisksDevice( mpa->partition() );
                QString type = device->property("PartitionType").toString();
                QString oldLabel = device->property("PartitionLabel").toString();
                QString newLabel = mpa->isLabelChanged() ? mpa->label() : oldLabel;
                
                success = device->modifyPartition(type, newLabel, mpa->flags());
                break;
            }
            
            case Action::ResizePartition: {
                ResizePartitionAction* rpa = dynamic_cast< ResizePartitionAction* >(action);
                device = new UDisksDevice( rpa->partition() );
                
                QString oldType = device->property("PartitionType").toString();
                QString oldLabel = device->property("PartitionLabel").toString();
                QStringList oldFlags = device->property("PartitionFlags").toStringList();
                
                success = device->deletePartition();
                device->deleteLater();
                
                QPair<VolumeTree, DeviceModified* > pair = d->map.searchTreeWithDevice( rpa->partition() );
                DeviceModified* disk = pair.first.root();
                device = new UDisksDevice( disk->name() );
                
                QDBusObjectPath p = device->createPartition(rpa->newOffset(), rpa->newSize(), oldType, oldLabel, oldFlags);
                
                if (p.path().isEmpty()) {
                    success = false;
                }
                
                qDebug() << "nuovo nome: " << p.path();
                break;
            }
            
            case Action::CreatePartitionTable: {
                CreatePartitionTableAction* cpta = dynamic_cast< CreatePartitionTableAction* >(action);
                
                device = new UDisksDevice( cpta->disk() );
                success = device->createTable( cpta->schemeName() );
                break;
            }
            
            case Action::RemovePartitionTable: {
                RemovePartitionTableAction* rpta = dynamic_cast< RemovePartitionTableAction* >(action);
                
                device = new UDisksDevice( rpta->disk() );
                success = device->createTable( "none" );
                break;
            }
            
            default:
                break;
        }
        
        device->deleteLater();
        
        if (!success) {
            d->error.setType(Utils::PartitioningError::ExecutionError);
            d->error.arg( action->description() );
            d->error.arg( device->latestError() );
            return false;
        }

        emit nextActionCompleted(++actionCount);
    }
    
    return true;
}

void ActionExecuter::Private::translateFutureNames(QList< Action* >::iterator currentpos, const QString& newPartitionName)
{
    for (QList< Action* >::iterator it = currentpos + 1; it != actions.end(); it++) {
        Action* current = (*it);
        
        if (partitionTypes.contains( current->actionType() )) {
            PartitionAction* pAction = dynamic_cast< PartitionAction* >(current);
            pAction->setPartitionName(newPartitionName);
        }
    }
}

}
}