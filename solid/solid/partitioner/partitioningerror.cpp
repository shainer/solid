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

#include <solid/partitioner/partitioningerror.h>
#include <QtCore/QObject>
#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{

PartitioningError::PartitioningError(PartitioningError::ErrorType type)
    : d( new PartitioningErrorPrivate )
{
    setType(type);
}

PartitioningError::PartitioningError(const PartitioningError& other)
    : d( other.d )
{}

PartitioningError::~PartitioningError()
{}

PartitioningError::ErrorType PartitioningError::type() const
{
    return d->type;
}

void PartitioningError::setType(PartitioningError::ErrorType type)
{
    d->type = type;
    
    switch (type) {
        case None: {
            d->description = "Success.";
            d->markersLeft = 0;
            break;
        }
        
        case PartitionNotFoundError: {
            d->description = "Partition with name \"%0\" not found.";
            d->markersLeft = 1;
            break;
        }
        
        case DiskNotFoundError: {
            d->description = "Disk with name \"%0\" not found.";
            d->markersLeft = 1;
            break;
        }
        
        case PartitionGeometryError: {
            d->description = "There is no suitable free space to create a new partition with offset %0 and size %1.";
            d->markersLeft = 2;
            break;
        }
        
        case ResizingToZeroError: {
            d->description = "Could not change a partition to have 0 size.";
            d->markersLeft = 0;
            break;
        }
        
        case ResizingToTheSameError: {
            d->description = "Both new offset and new size for %0 match the previous value.";
            d->markersLeft = 1;
            break;
        }
        
        case SafeResizingError: {
            d->description = "Safe resizing is impossible: the offset is changed, the size is too small, or the filesystem isn't supported.";
            d->markersLeft = 0;
            break;
        }
        
        case ExtendedResizingError: {
            d->description = "It's not allowed to resize an extended partition.";
            d->markersLeft = 0;
            break;
        }
        
        case ResizeOutOfBoundsError: {
            d->description = "Resizing out of bounds.";
            d->markersLeft = 0;
            break;
        }
        
        case ExceedingPrimariesError: {
            d->description = "Disk %0 has reached the limit of 4 primary partitions.";
            d->markersLeft = 1;
            break;
        }
        
        case ExceedingGPTPartitionsError: {
            d->description = "Disk %0 has reached the limit of 128 partitions.";
            d->markersLeft = 1;
            break;
        }
        
        case FilesystemError: {
            d->description = "The filesystem %0 isn't supported by UDisks, or cannot be created on a partition.";
            d->markersLeft = 1;
            break;
        }
        
        case FilesystemLabelError: {
            d->description = "The filesystem %0 doesn't support a label, or the maximum label length was exceeded.";
            d->markersLeft = 1;
            break;
        }

        case FilesystemFlagsError: {
            d->description = "The following flags don't exist for this filesystem type: %0";
            d->markersLeft = 1;
            break;
        }
        
        case FilesystemMinSizeError: {
            d->description = "Partition is too small for the %0 filesystem.";
            d->markersLeft = 1;
            break;
        }
        
        case MBRLabelError: {
            d->description = "The MBR scheme doesn't support labeled partitions.";
            d->markersLeft = 0;
            break;
        }
        
        case LabelTooBigError: {
            d->description = "The GPT scheme allows label only up to 36 characters.";
            d->markersLeft = 0;
            break;
        }
        
        case PartitionTooSmallError: {
            d->description = "The partition you wish to create or resize is below the minimum size allowed.";
            d->markersLeft = 0;
            break;
        }

        case PartitionFlagsError: {
            d->description = "The following flag isn't supported in this partitioning scheme: %0 (or you're trying to set it to an extended partition).";
            d->markersLeft = 1;
            break;
        }

        case NoPartitionTableError: {
            d->description = "Cannot create a partition on disk %0, which is without partition table";
            d->markersLeft = 1;
            break;
        }
        
        case CannotFormatPartition: {
            d->description = "The partition %0 cannot be formatted.";
            d->markersLeft = 1;
            break;
        }
        
        case BadPartitionTypeError: {
            d->description = "Cannot create a new partition of the requested type.";
            d->markersLeft = 0;
            break;
        }
        
        case MountedPartitionError: {
            d->description = "Cannot apply the action on mounted partition %0.";
            d->markersLeft = 1;
            break;
        }
        
        case RemovingExtendedError: {
            d->description = "Cannot remove the extended partition as it has at least one logical.";
            d->markersLeft = 0;
            break;
        }
        
        case IgnoredPartitionError: {
            d->description = "Cannot register an action on %0: this partition must be ignored by applications.";
            d->markersLeft = 1;
            break;
        }
        
        case DuplicateActionError: {
            d->description = "The action \"%0\" was already registered.";
            d->markersLeft = 1;
            break;
        }
        
        case BusyExecuterError: {
            d->description = "The executer is being used by another application.";
            d->markersLeft = 0;
            break;
        }
        
        case ExecutionError: {
            d->description = "An error occurred while executing the action \"%0\": %1.";
            d->markersLeft = 2;
            break;
        }
        
        default:
            break;
    }
}

void PartitioningError::arg(const QString& value)
{
    if (d->markersLeft == 0) {
        return;
    }
    
    d->description = d->description.arg(value);
    d->markersLeft--;
}

QString PartitioningError::description() const
{
    if (d->markersLeft > 0) {
        return QString();
    }
    
    return QObject::tr(d->description.toUtf8().data());
}
    
}
}
