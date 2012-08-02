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
#include <solid/partitioner/resizehelper.h>
#include <solid/partitioner/externalcommand.h>

#include <parted/parted.h>
#include <parted/filesys.h>

#define MEGABYTE 1024*1024

namespace Solid
{
namespace Partitioner
{

ActionReply ResizeHelper::minsize(QVariantMap arguments)
{
    QStringList notSupported = QStringList() << "NILFS2" << "BTRFS" << "Minix";
    QString filesystem = arguments["filesystem"].toString();
    QString diskName = udiToName( arguments["disk"].toString() );
    QString partitionName = udiToName( arguments["partition"].toString() );
    
    /*
     * If this flag is set to false, the partition filesystem isn't the one actually present on the disk, but a new one
     * requested to the partitioner. When it will be created, it will be empty, so in that case
     * we just return the minimum filesystem's size.
     */
    bool isOriginalFs = arguments["isOriginal"].toBool();

    ActionReply finalReply = ActionReply::SuccessReply; /* we always "succeed" */
    QVariantMap returnValues;
    
    /*
     * Note that for swap space all the resizing problem is irrelevant, as we don't need to save data. Thus a simple
     * value of 0 is returned.
     */
    if (filesystem == "unformatted" || filesystem.isEmpty() || filesystem == "Swap Space") {
        returnValues["minimumPartitionSize"] = 0;
    }
    else if (notSupported.contains(filesystem)) {
        returnValues["minimumPartitionSize"] = -1; /* resizing isn't supported for these ones */
    }
    else if (!isOriginalFs) {
        returnValues["minimumPartitionSize"] = arguments["minimumFilesystemSize"].toLongLong();
    }
    else if (filesystem == "FAT") {
        returnValues["minimumPartitionSize"] = minSizeParted(diskName, partitionName);
    }
    else {        
        returnValues["minimumPartitionSize"] = minSizeTool(partitionName, filesystem);
    }
    
    finalReply.setData(returnValues);
    return finalReply;
}

/* TODO: implement */
ActionReply ResizeHelper::resize(QVariantMap arguments)
{
    return ActionReply::SuccessReply;
}

QString ResizeHelper::udiToName(const QString& udi)
{
    QString name = udi.split("/").last();
    name.prepend("/dev/");
    return name;
}

qlonglong ResizeHelper::minSizeParted(const QString& diskName, const QString& partitionName)
{
    qlonglong minSize = -1;
    int partitionNumber = partitionName.right(1).toInt();
    ped_device_probe_all();

    PedDevice* device = ped_device_get( diskName.toUtf8().data() );
    PedDisk* disk = ped_disk_new(device);
    PedGeometry partitionGeometry = ped_disk_get_partition(disk, partitionNumber)->geom;
    PedFileSystem* partitionFilesystem = ped_file_system_open(&partitionGeometry);
    
    if (partitionFilesystem) {
        minSize = ped_file_system_get_resize_constraint(partitionFilesystem)->min_size;
    }
    
    ped_device_destroy(device);
    ped_disk_destroy(disk);
    return minSize;
}

qlonglong ResizeHelper::minSizeTool(const QString& partitionName, const QString& filesystem)
{
    QStringList commandLine;
    QRegExp numBlocksReg("unused");
    QRegExp blockSizeReg("unused");
    QRegExp freeBlocksReg("unused");
    qlonglong numBlocks = 0;
    qlonglong freeBlocks = 0;
    qlonglong blockSize = 1;
    
    if (filesystem == "NTFS") {
        commandLine << "ntfsresize" << "--info" << "--force" << "--no-progress-bar" << partitionName;
        
        freeBlocksReg = QRegExp("resize at (\\d+) bytes");
    }
    else if (filesystem.startsWith("Linux Ext")) {
        commandLine << "dumpe2fs" << "-h" << partitionName;
        
        numBlocksReg = QRegExp("Block count:\\s*(\\d+)");
        freeBlocksReg = QRegExp("Free blocks:\\s*(\\d+)");
        blockSizeReg = QRegExp("Block size:\\s*(\\d+)");
    }
    else if (filesystem == "ReiserFS") {
        commandLine << "debugreiserfs" << partitionName;
        
        numBlocksReg = QRegExp("Count of blocks[^:]+: (\\d+)");
        freeBlocksReg = QRegExp("Free blocks[^:]+: (\\d+)");
        blockSizeReg = QRegExp("Blocksize: (\\d+)");
    }
    else if (filesystem == "XFS") {
        commandLine << "xfs_db" << "-c" << "sb 0" << "-c" << "print" << partitionName;
        
        numBlocksReg = QRegExp("dblocks = (\\d+)");
        freeBlocksReg = QRegExp("fdblocks = (\\d+)");
        blockSizeReg = QRegExp("blocksize = (\\d+)");
    }
    
    ExternalCommand command(commandLine.takeFirst(), commandLine);
    bool started = command.run();
    
    if (!started || command.exitCode() != 0) {
        return -1;
    }
    
    if (numBlocksReg.indexIn( command.output() ) != -1) {
        numBlocks = numBlocksReg.cap(1).toLongLong();
    }
    if (freeBlocksReg.indexIn( command.output() ) != -1) {
        freeBlocks = freeBlocksReg.cap(1).toLongLong();
    }
    if (blockSizeReg.indexIn( command.output() ) != -1) {
        blockSize = blockSizeReg.cap(1).toLongLong();
    }
    
    qlonglong minSize = (numBlocks - freeBlocks) * blockSize;
    return (minSize > 0) ? minSize : -minSize; /* the sign change is needed for NTFS, as the min size is in freeBlocks */
}

}
}

KDE4_AUTH_HELPER_MAIN("org.solid.partitioner.resize", Solid::Partitioner::ResizeHelper)
