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
#ifndef SOLID_PARTITIONER_VOLUMETREEMAP_H
#define SOLID_PARTITIONER_VOLUMETREEMAP_H

#include <QtCore/QMap>
#include <solid/partitioner/volumetree.h>
#include <solid/partitioner/devices/disk.h>

namespace Solid
{
    namespace Partitioner
    {
        /**
         * @class VolumeTreeMap
         * @brief This class manages a map of trees representing disk layouts.
         * 
         * @author Lisa Vitolo <shainer@chakra-project.org>
         */
        class SOLID_EXPORT VolumeTreeMap : public QObject
        {
            Q_OBJECT

        public:
            /**
             * Creates an empty tree map.
             */
            explicit VolumeTreeMap();
            
            /**
             * Copy constructor.
             */
            VolumeTreeMap(const VolumeTreeMap &);
            virtual ~VolumeTreeMap();
            
            /**
             * Builds all the trees, detecting devices.
             */
            void build();
            
            /**
             * We internally store two copies of the map. One is modified as the application registers new actions,
             * while the other isn't; thus the latter always maintains the actual state of the hardware.
             * 
             * This method synchronizes the modified copy with the original one; it's used when undoing actions.
             */
            void backToOriginal();
            
            /**
             * Receive signals from the hardware about added/removed devices.
             */
            void connectSignals();
            
            /**
             * Receive signals from the hardware about added/removed devices.
             * This is used when executing actions to avoid being flooded with useless signals.
             */
            void disconnectSignals();
            
            /**
             * Retrieves the map of trees.
             * 
             * @returns a map of couples in the form (disk name -> disk layout tree).
             */
            QMap<QString, VolumeTree> deviceTrees() const;
            
            /**
             * Selection operator.
             * 
             * @param diskName the name of a disk.
             * @returns the correspondent tree object, or an invalid object if it doesn't exist.
             */
            VolumeTree operator[](const QString &) const;
            
            /**
             * Searches the map for a tree which contains the specified device.
             * 
             * @param devName the device name.
             * @returns a pair containing the device object and the tree in which it was found.
             */
            QPair<VolumeTree, Devices::DeviceModified *> searchTreeWithDevice(const QString &) const;
            
            /**
             * Searches the map for a tree which contains the specified partition.
             * 
             * @param devName the partition name.
             * @returns a pair containing the partition object and the tree in which it was found.
             */
            QPair<VolumeTree, Devices::Partition *> searchTreeWithPartition(const QString &) const;
            
            /**
             * Searches for a device inside the map.
             * 
             * @param udi the device UDI.
             * @returns the device object, or NULL if not found.
             */
            Devices::DeviceModified* searchDevice(const QString &) const;
            
            /**
             * Searches a partition inside the map.
             * 
             * @param udi the partition identifier.
             * @returns the partition object, or NULL if there isn't a partition with this name.
             */
            Devices::Partition* searchPartition(const QString &) const;
            
            /**
             * Searches the map for a disk layout.
             * 
             * @param diskName a disk's name.
             * @returns true if there's a layout tree for the disk, false otherwise.
             */
            bool contains(const QString &) const;
            
            /**
             * Removes a tree from the map.
             * 
             * @param diskName the UDI of the disk layout to remove.
             */
            void remove(const QString &);
            
            /**
             * Clears the map and destroys all the trees.
             */
            void clear();
            
        public slots:
            void doDeviceAdded(QString);
            void doDeviceRemoved(QString);
            
        signals:
            
            /**
             * This signal is emitted when a new disk or partition is added to the system.
             * 
             * @param tree the updated disk layout's tree.
             */
            void deviceAdded(VolumeTree);
            
            /**
             * This signal is emitted when a disk or partition is removed from the system.
             * 
             * @param udi the device UDI.
             * @param parentUdi the UDI of the disk on which this device is found.
             * @note if the deleted device is a disk, the two params contain the same value.
             */
            void deviceRemoved(QString, QString);
            
        private:
            class Private;
            Private* d;
        };
    }
}

#endif