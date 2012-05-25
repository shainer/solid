#include "resizepartitionaction.h"

namespace Solid
{
namespace Partitioner
{
namespace Actions
{

ResizePartitionAction::ResizePartitionAction(const QString& partition, qlonglong newOffset, qlonglong newSize)
    : m_partition(partition)
    , m_newOffset(newOffset)
    , m_newSize(newSize)
{}

ResizePartitionAction::ResizePartitionAction(const QString& partition, qlonglong newSize)
    : m_partition(partition)
    , m_newOffset(-1)
    , m_newSize(newSize)
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

qlonglong ResizePartitionAction::newOffset() const
{
    return m_newOffset;
}

qlonglong ResizePartitionAction::newSize() const
{
    return m_newSize;
}

}
}
}