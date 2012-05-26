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
                Unused,
                Primary,
                Logical,
                Extended
            };

            enum PTableType
            {
                None,
                MBR,
                GPT,
                APM
            };

        }
    }
}

#endif
