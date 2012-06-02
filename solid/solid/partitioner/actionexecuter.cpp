#include <solid/partitioner/actionexecuter.h>
#include "volumemanager.h"
#include <kglobal.h>

#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{

ActionExecuter::ActionExecuter()
    : QObject()
{
    m_actions = VolumeManager::instance()->registeredActions();
    m_disks = VolumeManager::instance()->allDiskTrees();
}

ActionExecuter::~ActionExecuter()
{}

bool ActionExecuter::execute()
{
    return true;
}

}
}