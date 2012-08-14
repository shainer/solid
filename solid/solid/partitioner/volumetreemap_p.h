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
#ifndef SOLID_PARTITIONER_VOLUMETREEMAP_PH
#define SOLID_PARTITIONER_VOLUMETREEMAP_PH

#include <solid/partitioner/volumetreemap.h>
#include <solid/ifaces/devicemanager.h>
#include <solid/partitioner/volumemanager.h>
#include <solid/backends/udisks/udisksmanager.h>

namespace Solid
{
    namespace Partitioner
    {
        class VolumeTreeMap::Private
        {
        public:
            Private(VolumeTreeMap *);
            ~Private();
            
            VolumeTreeMap* q;
            QMap<QString, VolumeTree> devices;
            QMap<QString, VolumeTree> beginningCopies; /* the original copy */
            Ifaces::DeviceManager* backend; /* This manager sends us the notifications about added and removed devices. */
            
            /* Builds all the trees, detecting devices. */
            void build();
            
            /*
             * We internally store two copies of the map. One is modified as the application registers new actions,
             * while the other isn't; thus the latter always maintains the actual state of the hardware.
             * 
             * This method synchronizes the modified copy with the original one; it's used when undoing actions.
             */
            void backToOriginal();
            
            /*
             * Receive signals from the hardware about added/removed devices.
             */
            void connectSignals();
            
            /*
             * Receive signals from the hardware about added/removed devices.
             * This is used when executing actions to avoid being flooded with useless signals.
             */
            void disconnectSignals();

            /*
            * NOTE: while detecting, either because we have to build the map or because a new device was added/removed,
            * we must update both maps.
            */
            void buildDisk(Device &);
            Devices::Disk* addDisk(Device &);
            void detectChildrenOfDisk(const QString &);
            void detectPartitionsOfDisk(const QString &);
            void detectFreeSpaceOfDisk(const QString &);
            
            /* The following functions help detecting free space blocks between partitions */
            QList< Devices::FreeSpace* > freeSpaceOfDisk(const VolumeTree &);
            QList< Devices::FreeSpace* > findSpace(QList< VolumeTreeItem* >, DeviceModified *);
            Devices::FreeSpace* spaceBetweenPartitions(DeviceModified *, DeviceModified *, DeviceModified *);
            
            /* Clears the map, destroying all the trees */
            void clear();
            
            friend class VolumeManager;
        };
    }
}

#endif