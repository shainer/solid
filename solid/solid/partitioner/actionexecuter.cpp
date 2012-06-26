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
#include <solid/partitioner/utils/partitiontableutils.h>
#include <solid/partitioner/utils/utils.h>

#include <solid/partitioner/actions/formatpartitionaction.h>
#include <solid/partitioner/actions/createpartitiontableaction.h>
#include <solid/partitioner/actions/removepartitiontableaction.h>
#include <solid/partitioner/actions/removepartitionaction.h>
#include <solid/partitioner/actions/modifypartitionaction.h>
#include <solid/partitioner/actions/resizepartitionaction.h>

#include <ifaces/devicemanager.h>
#include <backends/udisks/udisksmanager.h>

#include <kglobal.h>
#include <QtCore/QDebug>
#include <QtDBus/QDBusConnection>

#define EXECUTION_SERVICE "org.kde.Solid.Partitioner.Execution"

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
        , connection("connection")
        , valid(true)
    {}
    
    ~Private()
    {}
    
    void translateFutureNames(QList< Action* >::iterator, const QString &, const QString &);
    
    QList< Action* > actions;
    VolumeTreeMap map;
    QDBusConnection connection;
    bool valid;
    Utils::PartitioningError error;
};
    
ActionExecuter::ActionExecuter(const QList< Action* >& actions, const VolumeTreeMap& map)
    : QObject()
    , d( new Private(actions, map) )
{
    /*
     * We register a dummy service on the bus to avoid having two executers calling UDisks services together.
     * If another instance try to register this same service, it will fail.
     * The service is unregistered when this executer is destroyed.
     * 
     * FIXME: gives permissions to register the service on the system bus.
     */
    d->connection = QDBusConnection::sessionBus();
    d->valid = d->connection.registerService(EXECUTION_SERVICE);
    
    if (!d->valid) {
        d->error.setType(PartitioningError::BusyExecuterError);
    }
}

ActionExecuter::~ActionExecuter()
{
    d->connection.unregisterService(EXECUTION_SERVICE);
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
    
    d->map.backToOriginal(); /* this is necessary to have correct information for some actions */
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
                QString dummyPartitionName = cpa->partitionName();
                
                /* Finds the correct type strings for the partition given the scheme used. */
                Filesystem fs = cpa->filesystem();
                QString type = (cpa->partitionType() == ExtendedPartition) ? "extended" : fs.name();
                QString typeUDisks = Utils::PartitionTableUtils::instance()->typeString(disk->partitionTableScheme(), type);
                
                QDBusObjectPath newPartition = device->createPartition(cpa->offset(), cpa->size(), typeUDisks, cpa->label(),
                                                                       cpa->flags(), QStringList(),
                                                                       fs.name(), fs.flags());
                
                /*
                 * Actions registered on this partition after this one used the temporary unique name we gave to the partition,
                 * since we don't know what name the system will assign at this stage. So if the partition was created
                 * successfully we have to change the partition in all the following actions concerning it.
                 */
                if (newPartition.path().isEmpty()) {
                    success = false;
                }
                else {
                    d->translateFutureNames(it, newPartition.path(), dummyPartitionName);
                }
                
                break;
            }
            
            case Action::RemovePartition: {
                RemovePartitionAction* rpa = dynamic_cast< RemovePartitionAction* >(action);
                QPair< VolumeTree, Partition* > pair = d->map.searchTreeWithPartition( rpa->partition() );
                
                /*
                 * If we are deleting an extended, automatically removes the logicals (UDisks doesn't do it, but
                 * instead fails)
                 */
                if (pair.second->partitionType() == ExtendedPartition) {
                    foreach (DeviceModified* l, pair.first.logicalPartitions()) {
                        UDisksDevice* logical = new UDisksDevice( l->name() );
                        success = logical->deletePartition();
                        
                        if (!success) {
                            logical->deleteLater();
                            break;
                        }
                        
                        logical->deleteLater();
                    }
                }
                
                device = new UDisksDevice( rpa->partition() );
                success = device->deletePartition();
                
                break;
            }
            
            case Action::ModifyPartition: {
                ModifyPartitionAction* mpa = dynamic_cast< ModifyPartitionAction* >(action);
                
                device = new UDisksDevice( mpa->partition() );
                Partition* partition = d->map.searchPartition( mpa->partition() );
                
                QString type = partition->partitionTypeString();
                QString oldLabel = partition->label();
                QString newLabel = mpa->isLabelChanged() ? mpa->label() : oldLabel;
                
                success = device->modifyPartition(type, newLabel, mpa->flags());
                break;
            }
            
            case Action::ResizePartition: {
                ResizePartitionAction* rpa = dynamic_cast< ResizePartitionAction* >(action);
                device = new UDisksDevice( rpa->partition() );
                
                Partition* partition = d->map.searchPartition( rpa->partition() );
                
                QString oldType = partition->partitionTypeString();
                QString oldLabel = partition->label();
                QStringList oldFlags = partition->flags();
                
                success = device->deletePartition();
                device->deleteLater();
                
                QPair<VolumeTree, DeviceModified* > pair = d->map.searchTreeWithDevice( rpa->partition() );
                Disk* disk = pair.first.disk();
                device = new UDisksDevice( disk->name() );
                
                QDBusObjectPath p = device->createPartition(rpa->newOffset(), rpa->newSize(), oldType, oldLabel, oldFlags);
                
                if (p.path().isEmpty()) {
                    success = false;
                }
                
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

void ActionExecuter::Private::translateFutureNames(QList< Action* >::iterator currentpos,
                                                   const QString& newPartitionName,
                                                   const QString& currentPartitionName)
{
    for (QList< Action* >::iterator it = currentpos + 1; it != actions.end(); it++) {
        Action* current = (*it);
        
        if (Utils::isPartitionAction(current)) {
            PartitionAction* pAction = dynamic_cast< PartitionAction* >(current);
            
            if (pAction->partition() == currentPartitionName) {
                pAction->setPartitionName(newPartitionName);
            }
        }
    }
}

}
}