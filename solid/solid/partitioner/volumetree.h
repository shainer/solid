#ifndef SOLID_VOLUMETREE_H
#define SOLID_VOLUMETREE_H

#include <solid/storagevolume.h>
#include <solid/storagedrive.h>
#include "devices/devicemodified.h"
#include "devices/storagevolumemodified.h"
#include <QtCore/QDebug>

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
         * This is an implicitly shared class.
         * 
         * @author Lisa Vitolo <shainer@chakra-project.org>
         */
        class VolumeTreeItem
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
             * Constructs a copy of the tree item.
             * 
             * @param other the item to copy.
             */
            VolumeTreeItem(const VolumeTreeItem &);
            
            /**
             * Destroys a tree item.
             */
            ~VolumeTreeItem();
            
            DeviceModified* volume() const;
            VolumeTreeItem* parent() const;
            QList<VolumeTreeItem *> children() const;
            bool operator==(const VolumeTreeItem &) const;
            
            void addChild(DeviceModified *);
            
        private:
            QSharedDataPointer<VolumeTreeItemPrivate> d;
        };
        
        class VolumeTree
        {        
        public:
            VolumeTree(VolumeTreeItem *);
            VolumeTree(DeviceModified *);
            VolumeTree() {}
            VolumeTree(const VolumeTree &);
            ~VolumeTree();
            
            VolumeTreeItem* rootNode() const;
            DeviceModified* root() const;
            
            VolumeTreeItem* extendedNode() const;

            void addNode(const QString &, DeviceModified *);
            void addNode(const QString &, DeviceModified *, VolumeTreeItem *);
            void print() const;
            
        private:
            QSharedDataPointer<VolumeTreePrivate> d;
        };
        
        class VolumeTreeItemPrivate : public QSharedData
        {
        public:
            VolumeTreeItemPrivate(DeviceModified *, QList< VolumeTreeItem *>, VolumeTreeItem *);
            VolumeTreeItemPrivate(const VolumeTreeItemPrivate& other);
            ~VolumeTreeItemPrivate();
            
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

            void print(VolumeTreeItem *) const;
            void destroy(VolumeTreeItem *);
            
            VolumeTreeItem* root;
        };
        
    }
}

#endif