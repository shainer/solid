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
             * This method includes both the operations supported by this helper: computing the partition's minimum size and
             * actually resizing it. We only have the one method because since the application often needs to call both
             * in the same execution, we avoid having to ask twice for the root's password.
             * 
             * @param minSize true if computing minimum size, false for resizing
             * @param filesystem the filesystem's name
             * @param disk the disk's UDI
             * @param partition the partition's UDI
             * @param path the PATH environment variable.
             * 
             * Only for resizing:
             * @param newSize the new partition's size in bytes
             * @param oldSize the current partition's size in bytes.
             */
            ActionReply resizehelper(QVariantMap);
            
        private:
            /*
             * Translates a UDI into a device name.
             */
            QString udiToName(const QString &);
            
            qulonglong minsize(const QVariantMap &);
            QString resize(const QVariantMap &);
            
            /*
             * Some filesystem can be supported using libparted (root permissions are still required), namely FAT32.
             * The parameters are the disk name and the partition name.
             */
            qlonglong minSizeParted(const QString &, const QString &);
            
            /*
             * For the other filesystems, specific tools have to be invoked and their output analyzed.
             * The parameters here are the partition name, the filesystem name, and the PATH environment variable.
             */
            qlonglong minSizeTool(const QString &, const QString &, const QString &);
            
            QString resizePartition(const QString &, const QString &, qulonglong);
            QString resizeFilesystem(const QString &, const QString &, const QString &, qulonglong, const QString &);
        };
    }
}

#endif