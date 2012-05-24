#include "removepartitionaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

RemovePartitionAction::RemovePartitionAction(const QString& partition)
    : m_partition(partition)
{}

RemovePartitionAction::~RemovePartitionAction()
{}

Action::ActionType RemovePartitionAction::actionType() const
{
    return Action::RemovePartition;
}

QString RemovePartitionAction::partition() const
{
    return m_partition;
}
    
}
}
}