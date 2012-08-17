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
#ifndef SOLID_PARTITIONER_PARTITIONINGERROR_H
#define SOLID_PARTITIONER_PARTITIONINGERROR_H

#include <QtCore/QString>
#include <QtCore/QSharedDataPointer>
#include <solid/solid_export.h>

namespace Solid
{
    namespace Partitioner
    {
        class PartitioningErrorPrivate;
        
        /**
         * @class PartitioningError
         * @brief This class represents an error in the partitioning submodule.
         */
        class SOLID_EXPORT PartitioningError
        {
        public:

            /**
             * @enum ErrorType
             * @brief The error type.
             */
            enum ErrorType
            {
                None,
                PartitionNotFoundError,
                DiskNotFoundError,
                PartitionGeometryError,
                ResizingToZeroError,
                ResizingToTheSameError,
                SafeResizingError,
                ExtendedResizingError,
                ResizeOutOfBoundsError,
                ExceedingPrimariesError,
                ExceedingGPTPartitionsError,
                FilesystemError,
                FilesystemLabelError,
                FilesystemFlagsError,
                FilesystemMinSizeError,
                PartitionFlagsError,
                MBRLabelError,
                LabelTooBigError,
                PartitionTooSmallError,
                CannotFormatPartition,
                NoPartitionTableError,
                BadPartitionTypeError,
                MountedPartitionError,
                RemovingExtendedError,
                IgnoredPartitionError,
                DuplicateActionError,
                BusyExecuterError,
                ExecutionError
            };

            /**
             * Constructs a new object.
             *
             * @param type the error type, default to None.
             */
            PartitioningError(ErrorType type = None);
            PartitioningError(const PartitioningError &);
            virtual ~PartitioningError();

            /**
             * Error description have parameters to build more specific descriptions.
             * This method replaces the next marker with a string. If there were no parameters left
             * to replace, nothing happens.
             *
             * @param value the string to replace.
             */
            void arg(const QString &);

            /**
             * Retrieves a localized description of the error.
             *
             * @returns a localized description of the error if all parameters were replaced,
             * an empty string otherwise.
             */
            QString description() const;

            /**
             * @returns the error type.
             */
            ErrorType type() const;

            /**
             * Set a type for this error.
             *
             * @param type the error type.
             */
            void setType(ErrorType);
            
        private:
            QSharedDataPointer<PartitioningErrorPrivate> d;
        };
        
        class PartitioningErrorPrivate : public QSharedData
        {
        public:
            PartitioningErrorPrivate()
            {}
            
            PartitioningErrorPrivate(const PartitioningErrorPrivate& other)
                : QSharedData(other)
                , type(other.type)
                , description(other.description)
                , markersLeft(other.markersLeft)
            {}
            
            ~PartitioningErrorPrivate()
            {}
            
            PartitioningError::ErrorType type;
            QString description;
            int markersLeft;
        };
    }
}

#endif
