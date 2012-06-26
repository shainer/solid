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
#include "devices/partition.h"
#include "devices/freespace.h"
#include "devices/disk.h"

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
         * @brief This class represents an element in the tree of devices mantained by VolumeManager.
         * 
         * Each element in the tree represents a drive that can contain a partition table, a partition or a block
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
             * Destroys a tree item and its subtree recursively.
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
             * Adds a new child to the current device. The children are sorted in initial offset order.
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
             * Deletes all children of this item.
             */
            void clearChildren();
            
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
             * Constructs an invalid object.
             */
            VolumeTree();
            
            /**
             * Copy constructor.
             */
            VolumeTree(const VolumeTree &);
            
            /**
             * Destroys a tree and all the elements inside it.
             * All the device objects are destroyed too at this stage.
             */
            ~VolumeTree();
            
            /**
             * @returns whether the current object represents a valid tree.
             */
            bool valid() const;
            
            /**
             * @returns the root item.
             */
            VolumeTreeItem* rootNode() const;
            
            /**
             * @returns the root disk.
             */
            Disk* disk() const;
            
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
             * @returns a sorted list of primary and extended partitions.
             */
            QList< Partition* > partitions() const;
            
            /**
             * @returns a sorted list of logical partitions.
             */
            QList< Partition* > logicalPartitions() const;
            
            /**
             * @param parentName the name of the parent
             * @returns a sorted list of free space blocks
             */
            QList< FreeSpace* > freeSpaceBlocks(const QString& parentName) const;
            
            /**
             * @returns a list of all devices stored in this tree.
             */
            QList<DeviceModified *> allDevices() const;
            
            void print() const;
            
        private:
            friend class VolumeManager;
            friend class VolumeTreeMap;
            QSharedDataPointer<VolumeTreePrivate> d;
        };
        
        /*
         * FIXME: move these declarations.
         */
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
            VolumeTreePrivate();
            VolumeTreePrivate(const VolumeTreePrivate& other);
            ~VolumeTreePrivate();
            
            /*
             * Adds a new device given the parent name. 
             */
            void addDevice(const QString &, DeviceModified *);
            
            /*
             * Removes a device. A device inside a tree is uniquely identified by name.
             */
            void removeDevice(const QString &);
            
            /*
             * Removes all devices of a certain type.
             */
            void removeAllOfType(DeviceModified::DeviceModifiedType);
            
            /*
             * Deletes all the nodes in the tree.
             */
            void clear();
            
            /*
             * Searches a node inside the tree.
             */
            VolumeTreeItem* searchNode(const QString &) const;
            
            /*
             * When creating a new partition with given offset and size, finds the free space block that
             * can accomodate it (if present, otherwise returns NULL). Returns the correspondent tree item.
             */
            VolumeTreeItem* searchContainer(qulonglong offset, qulonglong size);
            
            /*
             * If the above method found a suitable container for the new partition, splits it around the partition.
             * New FreeSpace objects, which represents the free space left from the partition if any, are created and
             * added in the tree.
             * 
             * Parameters are offset and the size of the new partition; returns true if it was able
             * to find a suitable container (and that means the split happened), false otherwise.
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
            VolumeTreeItem* leftNode(DeviceModified *);
            
            /*
             * Returns the node immediately to the right of the passed one.
             * Remember nodes are sorted by initial offset.
             */
            VolumeTreeItem* rightNode(DeviceModified *);
            
            DeviceModified* leftDevice(DeviceModified *);
            DeviceModified* rightDevice(DeviceModified *);
            
            void print(VolumeTreeItem *) const;
            void destroy(VolumeTreeItem *);
            
            VolumeTreeItem* root;
            bool valid;
        };
        
    }
}

#endif