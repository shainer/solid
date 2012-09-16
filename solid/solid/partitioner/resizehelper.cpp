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
#include <QtCore/QDebug>

#include <parted/parted.h>
#include <parted/filesys.h>
#include <unistd.h>
#include <ktempdir.h>

#define MEGABYTE 1024*1024

namespace Solid
{
namespace Partitioner
{

ActionReply ResizeHelper::resizehelper(QVariantMap arguments)
{
    ActionReply finalReply = ActionReply::SuccessReply;  /* we always "succeed" */
    QVariantMap returnData;
    
    if (arguments["minSize"].toBool()) {
        returnData["minimumPartitionSize"] = minsize(arguments);
    }
    else {
        returnData["errorString"] = resize(arguments);
    }
    
    finalReply.setData(returnData);
    return finalReply;
}
    
qulonglong ResizeHelper::minsize(const QVariantMap& arguments)
{
    QString filesystem = arguments["filesystem"].toString();
    QString diskName = udiToName( arguments["disk"].toString() );
    QString partitionName = udiToName( arguments["partition"].toString() );
    
    if (filesystem == "FAT") {
        return minSizeParted(diskName, partitionName);
    }
    
    return minSizeTool(partitionName, filesystem, arguments["path"].toString());
}

QString ResizeHelper::resize(const QVariantMap& arguments)
{
    QString filesystem = arguments["filesystem"].toString();
    QString diskName = udiToName( arguments["disk"].toString() );
    QString partitionName = udiToName( arguments["partition"].toString() );
    QString path = arguments["path"].toString();
    
    qulonglong newSize = arguments["newSize"].toULongLong();
    qulonglong oldSize = arguments["oldSize"].toULongLong();
    bool expanding = newSize > oldSize;
    
    QString errorString;
    
    if (expanding || filesystem == "XFS") {
        errorString = resizePartition(diskName, partitionName, newSize);
        
        if (errorString.isEmpty()) {
            errorString = resizeFilesystem(diskName, partitionName, filesystem, newSize, path);
        }
    } else {
        errorString = resizeFilesystem(diskName, partitionName, filesystem, newSize, path);
        
        if (errorString.isEmpty()) {
            errorString = resizePartition(diskName, partitionName, newSize);
        }
    }
    
    return errorString;
}

QString ResizeHelper::resizePartition(const QString& diskName, const QString& partitionName, qulonglong size)
{
    qDebug() << "RESIZE PARTITION...";
    int partitionNum = partitionName.right(1).toInt();
    long long int sectorSize = size / 512;
    
    ped_device_probe_all();
    
    PedDevice* device = ped_device_get( diskName.toUtf8().data() );
    PedDisk* disk = ped_disk_new(device);
    PedPartition* partition = ped_disk_get_partition(disk, partitionNum);
    PedGeometry geometry = partition->geom;
    
    PedGeometry* newGeometry = ped_geometry_new(device, geometry.start, sectorSize);
    PedConstraint* constraint = ped_constraint_exact(newGeometry);
    
    if (!constraint) {
        return QString("Error while creating partition's constraint: check the new geometry.");
    }
    
    if (ped_disk_set_partition_geom(disk, partition, constraint, newGeometry->start, newGeometry->start + newGeometry->length - 1) == 0) {
        return QString("Error while setting the partition's new geometry");
    }
    
    ped_disk_commit_to_dev(disk);
    
    if (!ped_disk_commit_to_os(disk)) {
        sleep(1);
        ped_disk_commit_to_os(disk); /* NOTE: this is necessary because of a libparted bug */
    }
    
    qDebug() << "done";
    return QString();
}

QString ResizeHelper::resizeFilesystem(const QString& diskName,
                                    const QString& partition,
                                    const QString& filesystem,
                                    qulonglong size,
                                    const QString& path)
{
    QStringList commandLine;
    qDebug() << "RESIZE FILESYSTEM...";
    int exitCode = 0;
    
    if (filesystem == "NTFS") {
        ExternalCommand cmd("ntfsresize", QStringList() << "-P" << "-f" << partition << "-s" << QString::number(size), path);
        cmd.run();
        exitCode = cmd.exitCode();
    }
    else if (filesystem.startsWith("Linux Ext")) {
        QString sectors = QString::number(size / 512);
        sectors += "s";
        
        ExternalCommand cmd("resize2fs", QStringList() << partition << sectors, path);
        cmd.run();
        exitCode = cmd.exitCode();
    }
    else if (filesystem == "ReiserFS") {
        ExternalCommand cmd("resize_reiserfs", QStringList() << partition << "-q" << "-s" << QString::number(size), path);
        
        cmd.start();
        cmd.write("y\n", 2);
        cmd.waitFor();
        exitCode = cmd.exitCode();
    }
    else if (filesystem == "XFS") {
        KTempDir tempDir;
        
        if (!tempDir.exists()) {
            exitCode = 1;
        }
        
        ExternalCommand mount("mount", QStringList() << "-v" << "-t" << "xfs" << partition << tempDir.name(), path);
        if (!mount.run()) {
            exitCode = 1;
        }
        
        ExternalCommand grow("xfs_growfs", QStringList() << tempDir.name(), path);
        grow.run();
        
        ExternalCommand umount("umount", QStringList() << tempDir.name(), path);
        umount.run();
        
        exitCode = 0;
    }
    else if (filesystem == "FAT") {
        int partitionNum = partition.right(1).toInt();
        ped_device_probe_all();
        
        PedDevice* device = ped_device_get( diskName.toUtf8().data() );
        PedDisk* disk = ped_disk_new(device);
        PedGeometry geometry = ped_disk_get_partition(disk, partitionNum)->geom;
        PedFileSystem* partitionFilesystem = ped_file_system_open(&geometry);
        
        PedGeometry newGeometry;
        newGeometry.dev = device;
        newGeometry.start = geometry.start;
        newGeometry.length = size / 512;
        newGeometry.end = newGeometry.start + newGeometry.length;
        
        /* For LibParted 0 means failure, while for the filesystem tools it means success, so we change it */
        if (ped_file_system_resize(partitionFilesystem, &newGeometry, 0) == 0) {
            exitCode = 1;
        }
        
        ped_disk_commit_to_dev(disk);
        ped_disk_commit_to_os(disk);
        
        ped_disk_destroy(disk);
        ped_device_destroy(device);
    }
    
    qDebug() << "done.";
    
    if (exitCode == 0) {
        return QString();
    }
    
    return QString("Error while resizing the filesystem.");
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

qlonglong ResizeHelper::minSizeTool(const QString& partitionName, const QString& filesystem, const QString& envPath)
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
    
    ExternalCommand command(commandLine.takeFirst(), commandLine, envPath);
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
