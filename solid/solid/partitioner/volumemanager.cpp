#include "volumemanager.h"
#include "devices/storagevolumemodified.h"
#include "devices/storagedrivemodified.h"
#include <kglobal.h>
#include <solid/storagedrive.h>
#include <solid/deviceinterface.h>
#include <QtCore/QDebug>
#include <solid/device.h>

namespace Solid
{   
namespace Partitioner
{
    
class VolumeManagerHelper
{
public:
    VolumeManagerHelper()
        : q(0)
    {}
    
    virtual ~VolumeManagerHelper()
    {
        delete q;
    }
    
    VolumeManager* q;
};

K_GLOBAL_STATIC(VolumeManagerHelper, s_volumemanager);

VolumeManager::VolumeManager()
    : executer(0)
{
    Q_ASSERT(!s_volumemanager->q);
    s_volumemanager->q = this;
    
    detectDevices();
}

VolumeManager::~VolumeManager()
{}

VolumeManager* VolumeManager::instance()
{
    if (!s_volumemanager->q) {
        new VolumeManager;
    }
    
    return s_volumemanager->q;
}

void VolumeManager::detectDevices()
{
    foreach(Device dev, Device::listFromType(DeviceInterface::StorageDrive)) {
        QString udi = dev.udi();
        Solid::StorageDrive *drive = dev.as<Solid::StorageDrive>();
        
        if (drive->driveType() == StorageDrive::HardDisk)
        {
            Devices::StorageDriveModified* driveModified = new Devices::StorageDriveModified( drive );
            driveModified->setUdi(udi);
            
            VolumeTree tree( driveModified );
            volumeTrees.insert(udi, tree);
        }
    }
    
    QMap<qulonglong, Devices::StorageVolumeModified *> partitions;
    foreach(Device dev, Device::listFromType(DeviceInterface::StorageVolume)) {
        StorageVolume* volume = dev.as<StorageVolume>();
        
        Devices::StorageVolumeModified* volumeModified = new Devices::StorageVolumeModified(volume);
        volumeModified->setUdi(dev.udi());
        volumeModified->setParentUdi(dev.parentUdi());
        
        partitions.insert(volumeModified->offset(), volumeModified);
    }
    
    Devices::StorageVolumeModified* extended = NULL;
    
    foreach (Devices::StorageVolumeModified* volume, partitions.values()) {
        QString parentUdi = volume->parentUdi();
        
        if (!volumeTrees.contains(parentUdi)) {
            continue;
        }
        
        VolumeTree tree = volumeTrees.value(parentUdi);
        
        if (volume->uuid().isEmpty()) {
            extended = volume;
            extended->setPartitionType(StorageVolumeModified::Extended);
        }
        else if (extended && (volume->offset() >= extended->offset() && volume->rightBoundary() <= extended->rightBoundary())) {
            volume->setPartitionType(StorageVolumeModified::Logical);
            parentUdi = extended->udi();
        }
        else {
            volume->setPartitionType(StorageVolumeModified::Primary);
        }
        
        tree.addNode(parentUdi, volume);
    }
}

bool VolumeManager::apply()
{
    executer = new ActionExecuter(actionstack.list());
    qDebug() << executer->valid();
    
    ActionExecuter* e2 = new ActionExecuter(actionstack.list());
    qDebug() << e2->valid();
    
    delete executer;
    delete e2;
    
    return true;
}

}
}