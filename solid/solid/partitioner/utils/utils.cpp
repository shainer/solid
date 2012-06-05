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

#include <solid/partitioner/utils/utils.h>
#include <solid/partitioner/devices/disk.h>

#include <QtCore/QStringList>

namespace Solid
{
namespace Partitioner
{
namespace Utils
{

using namespace Devices;
    
QList< FreeSpace* > findSpace(QList< Partitioner::VolumeTreeItem* >, DeviceModified*, bool logicals = false);
FreeSpace* spaceBetweenPartitions(Partition *, Partition *, DeviceModified *);
FreeSpace* spaceBetweenLogicalPartitions(Partition *, Partition *, DeviceModified *);

QList< FreeSpace* > freeSpaceOfDisk(const Solid::Partitioner::VolumeTree& diskTree)
{
    QList< VolumeTreeItem* > primaryPartitions = diskTree.rootNode()->children();
    Disk* disk = dynamic_cast< Disk* >(diskTree.root());
    VolumeTreeItem* extended = diskTree.extendedNode();
    
    QList< FreeSpace* > freeSpaces;
    
    freeSpaces.append( findSpace(primaryPartitions, disk) );
    
    if (extended) {
        freeSpaces.append( findSpace(extended->children(), extended->volume(), true) );
    }
    
    return freeSpaces;
}

QList< FreeSpace* > findSpace(QList< VolumeTreeItem* > partitions, DeviceModified* parent, bool logicals)
{
    QList< FreeSpace* > freeSpaces;
    
    if (partitions.isEmpty()) {
        freeSpaces.append( new FreeSpace(parent->offset(), parent->size(), parent->name()) );
        return freeSpaces;
    }
    
    for (int i = -1; i < partitions.size(); i++) {
        Partition* volume1 = NULL;
        Partition* volume2 = NULL;
        FreeSpace *space = NULL;
        
        if (i == -1) {
            volume2 = dynamic_cast< Partition* >(partitions[i+1]->volume());
        }
        else if (i < partitions.size() - 1) {
            volume1 = dynamic_cast< Partition* >(partitions[i]->volume());
            volume2 = dynamic_cast< Partition* >(partitions[i+1]->volume());
        }
        else {
            volume1 = dynamic_cast< Partition* >(partitions[i]->volume());
        }
        
        if (!logicals) {
            space = spaceBetweenPartitions(volume1, volume2, parent);
        } else {
            space = spaceBetweenLogicalPartitions(volume1, volume2, parent);
        }
        
        if (space) {
            freeSpaces.append(space);
        }
    }
    
    return freeSpaces;
}

FreeSpace* spaceBetweenPartitions(Partition* partition1,
                                                 Partition* partition2,
                                                 DeviceModified* disk)
{
    FreeSpace* sp = 0;
    
    if (!partition1) {
        qulonglong initialOffset = disk->offset();
        if (partition2->offset() > initialOffset) {
            sp = new FreeSpace(initialOffset, partition2->offset() - initialOffset, disk->name());
        }
    }
    else if (!partition2) {
        if (partition1->rightBoundary() < disk->size()) {
            sp = new FreeSpace(partition1->rightBoundary(), disk->size() - partition1->rightBoundary(), disk->name());
        }
    }
    else {
        if (partition1->rightBoundary() < partition2->offset()) {
            sp = new FreeSpace(partition1->rightBoundary(), (partition2->offset() - partition1->rightBoundary()), disk->name());
        }
    }
    
    return sp;
}

FreeSpace* spaceBetweenLogicalPartitions(Partition* partition1,
                                                        Partition* partition2,
                                                        DeviceModified* extended)
{
    FreeSpace* sp = 0;
    
    if (!partition1) {
        qulonglong e_offset = extended->offset() + SPACE_BETWEEN_LOGICALS;
        if (e_offset < partition2->offset()) {
            sp = new FreeSpace(e_offset, partition2->offset() - e_offset, extended->name());
        }
    }
    else if (!partition2) {
        qulonglong e_rightOffset = extended->offset() + extended->size();
        if (partition1->rightBoundary() < e_rightOffset) {
            sp = new FreeSpace(partition1->rightBoundary(), e_rightOffset - partition1->rightBoundary(), extended->name());
        }
    }
    else {
        if (partition1->rightBoundary() + SPACE_BETWEEN_LOGICALS < partition2->offset()) {
            sp = new FreeSpace(partition1->rightBoundary(), partition2->offset() - partition1->rightBoundary(), extended->name());
        }
    }
    
    return sp;
}

}
}
}