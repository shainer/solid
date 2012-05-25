#include "resizepartitionaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

ResizePartitionAction::ResizePartitionAction(const QString& partition, qulonglong newoffset, qulonglong newsize)
    : m_partition(partition)
    , m_newOffset(newoffset)
    , m_newSize(newsize)
{}

ResizePartitionAction::~ResizePartitionAction()
{}

Action::ActionType ResizePartitionAction::actionType() const
{
    return ResizePartition;
}

QString ResizePartitionAction::partition() const
{
    return m_partition;
}

qulonglong ResizePartitionAction::newOffset() const
{
    return m_newOffset;
}

qulonglong ResizePartitionAction::newSize() const
{
    return m_newSize;
}

}
}
}