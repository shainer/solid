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
             * @pre the action cannot be null.
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
             * Retrieves the layout of a disk.
             * 
             * @param udi the disk UDI.
             * @returns the layout tree, or an invalid object if not found.
             */
            VolumeTree diskTree(const QString &) const;
            
            /**
             * @returns all the disk layouts.
             * @see VolumeTreeMap
             */
            VolumeTreeMap allDiskTrees() const;
            
            /**
             * Retrieves the latest error occurred.
             * 
             * @returns an error object, with None as type for success.
             */
            PartitioningError error() const;
            
            /**
             * Retrieves the list of registered actions.
             * 
             * @returns a list of actions.
             */
            QList< Action* > registeredActions() const;
            
        public slots:
            /**
             * Slot called when a new device is added to the system.
             * @param udi the new device's UDI.
             */
            void doDeviceAdded(QString);
            
            /**
             * Slot called when a device is removed from the system.
             * @param udi the device's UDI.
             */
            void doDeviceRemoved(QString);
            
            /**
             * Slot called when the applying of a new action was completed.
             * @param index the new action's index within the registered list, starting from 1.
             */
            void doNextActionCompleted(int);
            
        signals:
            
            /**
             * This signal is emitted whenever a disk layout changes, not in the hardware, but in our internal representation.
             * 
             * @param name the disk name.
             */
            void diskChanged(QString);
            
            /**
             * This signal is emitted whenever the accessibility of one volume changes. Applications are thus advised to
             * read again information about the device to update their data.
             * 
             * @param accessible whether the volume is now accessible.
             * @param udi the volume UDI.
             */
            void accessibilityChanged(bool, const QString &);
            
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
            
            /**
             * This signal reports that the execution finished successfully AND the new device state has been detected.
             */
            void executionFinished();
            
        private:
            VolumeManager();
            
            void connectAllPartitionAccessibility();
            
            class Private;
            Private* d;
        };

    }
}

#endif