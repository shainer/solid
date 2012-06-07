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
            VolumeTreeMap(const VolumeTreeMap &);
            virtual ~VolumeTreeMap();
            
            /**
             * Builds all the trees detecting devices.
             */
            void build();
            
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
             * @param devName the name of the device.
             * @returns a pair containing the device object and the tree in which it was found.
             */
            QPair<VolumeTree, Devices::DeviceModified *> searchTreeWithDevice(const QString &) const;
            
            /**
             * Searches a specific device inside the map.
             * 
             * @param udi the device UDI.
             * @returns the device object, or NULL if not found.
             */
            Devices::DeviceModified* searchDevice(const QString &) const;
            
            /**
             * Searches the map for a disk layout.
             * 
             * @param diskName the name of a disk.
             * @returns true if there's a layout tree for the disk, false otherwise.
             */
            bool contains(const QString &) const;
            
            /**
             * Removes a tree from the map.
             * 
             * @param diskName the name of the disk to remove.
             */
            void remove(const QString &);
            
            /**
             * Clears the map and destroys all the disks.
             */
            void clear();
            
        public slots:
            void doDeviceAdded(QString);
            void doDeviceRemoved(QString);
            
        signals:
            
            /**
             * This signal is emitted when a new device (disk or partition) is added to the system.
             * 
             * @param tree the updated disk layout's tree.
             */
            void deviceAdded(VolumeTree);
            
            /**
             * This signal is emitted when a device is removed from the system.
             * 
             * @param name the name of the disk where the device is contained, if any.
             */
            void deviceRemoved(QString);
            
        private:
            class Private;
            Private* d;
        };
    }
}

#endif