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
#ifndef SOLID_PARTITIONER_RESIZEHELPER_H
#define SOLID_PARTITIONER_RESIZEHELPER_H

#include <QtCore/QVariant>
#include <kauth.h>

namespace Solid
{
    namespace Partitioner
    {
        using namespace KAuth;
        
        /**
         * @class ResizeHelper
         * @extends QObject
         * @brief This class manages all resizing operations for a partition object.
         */
        class ResizeHelper : public QObject
        {
            Q_OBJECT
            
        public slots:            
            /**
             * Retrieves the minimum partition size we can set if we want to avoid crushing any data; -1 is returned if we don't
             * support resizing for the filesystem, or some error occurred during the execution.
             * The return value is a qlonglong, named "minimumPartitionSize".
             * 
             * @param partition (string) the partition UDI
             * @param disk (string) the disk UDI, needed by libparted
             * @param filesystem (string) the filesystem name
             * @param isOriginal (bool) whether the filesystem exists on the partition or was set later
             * @param minimumFilesystemSize (qulonglong) the filesystem's minimum size allowed.
             * The last one is required by the caller to avoid having to include FilesystemUtils as a dependency of the helper.
             * @note these parameters must be passed in a QVariantMap.
             * 
             * @returns the minimum partition size.
             */
            ActionReply minsize(QVariantMap);
            
            /**
             * @todo to be implemented.
             */
            ActionReply resize(QVariantMap);
            
        private:
            /*
             * Translates a UDI into a device name.
             */
            QString udiToName(const QString &);
            
            /*
             * Some filesystem can be supported using libparted (root permissions are still required), namely FAT32.
             * The parameters are the disk name and the partition name.
             */
            qlonglong minSizeParted(const QString &, const QString &);
            
            /*
             * For the other filesystems, specific tools have to be invoked and their output analyzed.
             * The parameters here are the partition name and the filesystem name.
             */
            qlonglong minSizeTool(const QString &, const QString &);
        };
    }
}

#endif