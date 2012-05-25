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

#ifndef SOLID_PARTITIONER_VOLUMETREE_H
#define SOLID_PARTITIONER_VOLUMETREE_H

#include <solid/storagevolume.h>
#include <solid/storagedrive.h>
#include <solid/solid_export.h>
#include <solid/partitioner/devices/devicemodified.h>
#include <solid/partitioner/devices/partition.h>

#include <QtCore/QDebug>
#include <QtCore/QSharedData>

namespace Solid
{
    namespace Partitioner
    {
    
        class VolumeTreeItemPrivate;
        class VolumeTreePrivate;
        
        using namespace Devices;
        
        /**
         * @class VolumeTreeItem
         * @brief This class represents an element in the tree of volumes mantained by VolumeManager.
         * 
         * Each element in the tree can represent a drive that can contain a partition table, a partition or a chunk
         * of free space inside a disk.
         * 
         * @author Lisa Vitolo <shainer@chakra-project.org>
         */
        class SOLID_EXPORT VolumeTreeItem
        {
        public:
            
            /**
             * Constructs a tree item.
             * 
             * @param dev the device or free space represented by this item.
             * @param children the list of children. The children are sorted by initial offset.
             * @param parent a pointer to the parent.
             */
            VolumeTreeItem(DeviceModified *, QList<VolumeTreeItem *> children = QList<VolumeTreeItem *>(), VolumeTreeItem* parent = 0);
            
            /**
             * Destroys a tree item.
             */
            ~VolumeTreeItem();
            
            /**
             * @returns the device associated with this item.
             */
            DeviceModified* volume() const;
            
            /**
             * @returns the parent of the current item (NULL if this is the root).
             */
            VolumeTreeItem* parent() const;
            
            /**
             * @returns the sorted list of children.
             */
            QList<VolumeTreeItem *> children() const;
            
            /**
             * Adds a new child to the current device. Order is maintained.
             * 
             * @param dev the device object to add.
             */
            void addChild(DeviceModified *);
            
            /**
             * Removes the given node from the children list.
             * 
             * @param child the node to remove.
             */
            void removeChild(VolumeTreeItem *);
            
            /**
             * Comparison operator.
             * 
             * @returns true if the two items represent the same device, false otherwise.
             */
            bool operator==(const VolumeTreeItem &) const;
            
        private:
            class Private;
            Private* d;
        };
        
        /**
         * @class VolumeTree
         * @brief This class represents a disk's layout.
         * 
         * The layout is composed by the partitions inside this disk and special devices representing
         * the blocks of free space inside the disk.
         * Logical partitions are inserted as children of the extended.
         * 
         * This is an implicitly shared class.
         * 
         * @author Lisa Vitolo <shainer@chakra-project.org>
         */
        class SOLID_EXPORT VolumeTree
        {        
        public:
            /**
             * Costructs a new tree.
             * 
             * @param rootItem the root item.
             */
            VolumeTree(VolumeTreeItem *);
            
            /**
             * Constructs a new tree.
             * 
             * @param rootDevice the root device.
             */
            VolumeTree(DeviceModified *);
            
            /**
             * Constructs an empty tree.
             */
            VolumeTree() {}
            
            /**
             * Copy constructor.
             */
            VolumeTree(const VolumeTree &);
            
            /**
             * Destroys a tree and all the elements inside it.
             * All the DeviceModified objects are destroyed too at this stage.
             */
            ~VolumeTree();
            
            /**
             * @returns the root item.
             */
            VolumeTreeItem* rootNode() const;
            
            /**
             * @returns the root device.
             */
            DeviceModified* root() const;
            
            /**
             * @returns the item representing the extended partition, or NULL if there isn't one.
             * @note each disk can have up to one extended partition.
             */
            VolumeTreeItem* extendedNode() const;
            
            /**
             * @returns the device object representing the extended partition, or NULL if there isn't one.
             * @note each disk can have up to one extended partition.
             */
            DeviceModified* extendedPartition() const;
            
            /**
             * Looks for a device in the tree.
             * 
             * @param name the name of the device (identifier).
             * @note all devices are identified by a name, even free space or partitions that don't exist in the system yet.
             * @returns the device object, or NULL if not found.
             */
            DeviceModified* searchDevice(const QString &) const;
            
            /**
             * Searches a node in the tree.
             * @param name the name of the device (identifier).
             * @note all devices are identified by a name, even free space or partitions that don't exist in the system yet.
             * @returns the node, or NULL if not found.
             */
            VolumeTreeItem* searchNode(const QString &) const;
            
            /**
             * @param free include blocks of free space too.
             * @returns a sorted list of primary and extended partitions.
             */
            QList<DeviceModified *> partitions(bool free = false) const;
            
            /**
             * @param free include blocks of free space too.
             * @returns a sorted list of logical partitions.
             */
            QList<DeviceModified *> logicalPartitions(bool free = false) const;

            /**
             * Adds a new device.
             * FIXME: check if the parent exists.
             * 
             * @param parentName the name of the parent.
             * @param device the new object to add.
             */
            void addDevice(const QString &, DeviceModified *);
            
            /**
             * Removes a device. A device inside a tree is uniquely identified by name.
             * FIXME: come prima.
             * 
             * @param deviceName the device name.
             */
            void removeDevice(const QString &);
            
            /**
             * Deletes all the nodes in the tree.
             */
            void clear();
            
            
            void print() const;
            
        private:
            friend class VolumeManager;
            QSharedDataPointer<VolumeTreePrivate> d;
        };
        
        class VolumeTreeItem::Private
        {
        public:
            Private(DeviceModified *, QList< VolumeTreeItem *>, VolumeTreeItem *);
            ~Private();
            
            DeviceModified* volume;
            QList<VolumeTreeItem *> children;
            VolumeTreeItem* parent;
        };
        
        class VolumeTreePrivate : public QSharedData
        {
        public:
            VolumeTreePrivate(VolumeTreeItem *);
            VolumeTreePrivate(const VolumeTreePrivate& other);
            ~VolumeTreePrivate();

            void addDevice(const QString &, DeviceModified *, VolumeTreeItem *);
            VolumeTreeItem* searchNode(const QString &) const;
            void removeDevice(const QString &, VolumeTreeItem *);
            
            /*
             * When a new partition is requested, checks if a suitable block of free space exists.
             * If this block is bigger than the new partition, splits it around the partition.
             * Parameters are offset and the size of the new partition; returns true if it was able
             * to find a suitable container (that means the split happened), false otherwise.
             */
            bool splitCreationContainer(qulonglong, qulonglong);
            
            /*
             * Deletes a partition from the disk: the space previously occupied by it will become a free space block.
             * This block is then merged with adjacent blocks, if they are present.
             */
            void mergeAndDelete(const QString &);
            
            /*
             * Returns the node immediately to the left of the passed one.
             * Remember nodes are sorted by initial offset.
             */
            VolumeTreeItem* leftSibling(VolumeTreeItem *);
            
            /*
             * Returns the node immediately to the right of the passed one.
             * Remember nodes are sorted by initial offset.
             */
            VolumeTreeItem* rightSibling(VolumeTreeItem *);
            
            DeviceModified* leftDevice(VolumeTreeItem *);
            DeviceModified* rightDevice(VolumeTreeItem *);
            
            void print(VolumeTreeItem *) const;
            void destroy(VolumeTreeItem *);
            
            VolumeTreeItem* root;
        };
        
    }
}

#endif