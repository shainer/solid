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

        }
    }
}

#endif
