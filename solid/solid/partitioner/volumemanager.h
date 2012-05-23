#ifndef SOLID_PARTITIONER_VOLUMEMANAGER_H
#define SOLID_PARTITIONER_VOLUMEMANAGER_H
#include <QtCore/QList>

#include <solid/solid_export.h>
#include <solid/partitioner/volumetree.h>
#include <solid/partitioner/actionstack.h>
#include <solid/partitioner/actionexecuter.h>

namespace Solid
{
    namespace Partitioner
    {

        class SOLID_EXPORT VolumeManager
        {
        public:
            virtual ~VolumeManager();
            static VolumeManager* instance();
            
            bool apply();
                    
        private:
            VolumeManager();
            void detectDevices();
            
            QMap<QString, VolumeTree> volumeTrees;
            ActionStack actionstack;
            ActionExecuter* executer;
        };

    }
}

#endif