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
        /**
         * @class VolumeManager
         * 
         * This class is the core of the partitioner submodule. It manages drives and partitions using trees and
         * allows the application to register and apply actions on them.
         * 
         * @author Lisa Vitolo <shainer@chakra-project.org>
         */
        class SOLID_EXPORT VolumeManager
        {
        public:
            virtual ~VolumeManager();
            static VolumeManager* instance();
            
            /**
             * Registers a new action for later execution. This means the requested changes are applied on the objects
             * representing drives and partitions, but not on the actual hardware.
             * 
             * @param action the action to register.
             * @returns true if the action was valid, false otherwise.
             */
            bool registerAction(Actions::Action *);
            
            /**
             * Undoes the last action registered.
             * If no action was registered, nothing happens.
             */
            void undo();
            
            /**
             * Redoes the last action undone.
             * If no action was undone, nothing happens.
             */
            void redo();
            
            /**
             * Applies a list of actions to the hardware.
             * @see class ActionExecuter.
             */
            bool apply();
            
            /**
             * Retrieves all the trees representing drives.
             * 
             * @returns a list of pointers to the disk tree managed internally by this class.
             */
            QList<VolumeTree> allDiskTrees() const;
            
            /**
             * Retrieves the tree representing a given drive.
             * 
             * @param diskName the disk name.
             * @returns a pointer of the associated tree.
             */
            VolumeTree diskTree(const QString &) const;
                    
        private:
            VolumeManager();
            
            /* Detection of drives, partitions, and free space */
            void detectDevices();
            
            /* Searches a device by its name in all trees. Returns NULL if not found. */
            DeviceModified* searchDeviceByName(const QString &);
            
            QMap<QString, VolumeTree> volumeTrees;
            ActionStack actionstack;
            ActionExecuter* executer;
        };

    }
}

#endif