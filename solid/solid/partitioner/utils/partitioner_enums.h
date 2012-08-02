#ifndef SOLID_PARTITIONER_ENUMS_H
#define SOLID_PARTITIONER_ENUMS_H

namespace Solid
{
    namespace Partitioner
    {
        namespace Utils
        {

            enum PartitionType
            {
                UnusedPartition,
                PrimaryPartition,
                LogicalPartition,
                ExtendedPartition
            };

            enum PartitionTableScheme
            {
                NoneScheme,
                MBRScheme,
                GPTScheme,
                APMScheme
            };
            
            /**
             * @enum FilesystemType
             * @brief Describes a filesystem's type
             */
            enum FilesystemType
            {
                Unformatted,
                NTFS,
                FAT,
                ReiserFS,
                Swap,
                NILFS2,
                BTRFS,
                Ext2,
                Ext3,
                Ext4,
                Minix,
                XFS
            };

        }
    }
}

#endif
