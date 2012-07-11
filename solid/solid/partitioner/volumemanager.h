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
#include <QtDBus/QDBusObjectPath>

#include <solid/solid_export.h>
#include <solid/partitioner/volumetree.h>
#include <solid/partitioner/actionstack.h>
#include <solid/partitioner/actionexecuter.h>
#include <solid/partitioner/volumetreemap.h>

namespace Solid
{
    namespace Partitioner
    {
        /**
         * @class VolumeManager
         * 
         * This singleton is the core of the partitioner submodule. It manages drives and partitions using trees and
         * allows the application to register and apply actions on them.
         * 
         * @author Lisa Vitolo <shainer@chakra-project.org>
         */
        class SOLID_EXPORT VolumeManager : public QObject
        {
            Q_OBJECT
            Q_DISABLE_COPY(VolumeManager)
        
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
             * If no action is registered, nothing happens.
             */
            void undo();
            
            /**
             * Redoes the last action undone.
             * If no action was undone, nothing happens.
             */
            void redo();
            
            /**
             * @returns true if there's at least one action to undo, false otherwise.
             */
            bool isUndoPossible() const;
            
            /**
             * @returns true if there's at least one action to redo, false otherwise.
             */
            bool isRedoPossible() const;
            
            /**
             * Applies a list of actions to the hardware.
             * @returns true for success, false otherwise.
             */
            bool apply();
            
            /**
             * Deletes all registered actions. This MUST be called after apply().
             * We cannot call it automatically to avoid dangling pointers in the applications.
             */
            void clearActions();
            
            /**
             * Retrieves the layout of a disk.
             * 
             * @param udi the disk UDI.
             * @returns the layout tree, or an invalid object if not found.
             */
            VolumeTree diskTree(const QString &) const;
            
            /**
             * @returns all the disk layouts in a map indexed by disk UDIs.
             */
            QMap<QString, VolumeTree> allDiskTrees() const;
            
            /**
             * Retrieves the latest error occurred.
             * 
             * @returns an error object, with None as type for success.
             */
            Utils::PartitioningError error() const;
            
            /**
             * Retrieves the list of registered actions.
             * 
             * @returns a list of actions.
             */
            QList< Action* > registeredActions() const;
            
        public slots:
            
            /**
             * @see class VolumeTreeMap.
             */
            void doDeviceAdded(VolumeTree);
            void doDeviceRemoved(QString, QString);
            void doNextActionCompleted(int);
            
        signals:
            
            /**
             * This signal is emitted whenever a disk layout changes, not in the hardware, but in our internal representation.
             * 
             * @param name the disk name.
             */
            void diskChanged(QString);
            
            /**
             * This signal is emitted when a new device (disk or partition) is added to the system.
             * 
             * @param tree the updated disk layout's tree.
             */
            void deviceAdded(VolumeTree);
            
            /**
             * This signal is emitted when a device is removed from the system.
             * 
             * @param udi the device UDI.
             */
            void deviceRemoved(QString);
            
            /**
             * This signal is used to report progress when executing a list of actions.
             * 
             * @param index the index of the action that was last executed successfully.
             */
            void progressChanged(int);
            
            /**
             * This signal reports an error in the execution of an action (after this, the entire execution stops).
             * 
             * @param error the error object.
             */
            void executionError(QString);
            
        private:
            VolumeManager();
            
            class Private;
            Private* d;
        };

    }
}

#endif