#include "formatpartitionaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{
    
FormatPartitionAction::FormatPartitionAction(const QString& partition, const QString& fs)
    : m_partition(partition)
    , m_filesystem(fs)
{}

FormatPartitionAction::~FormatPartitionAction()
{}

Action::ActionType FormatPartitionAction::actionType() const
{
    return Action::FormatPartition;
}

QString FormatPartitionAction::partition() const
{
    return m_partition;
}

QString FormatPartitionAction::filesystem() const
{
    return m_filesystem;
}

}
}
}