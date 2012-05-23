#include <solid/partitioner/actionexecuter.h>
#include <kglobal.h>

#include <QtCore/QDebug>

namespace Solid
{
namespace Partitioner
{

class ActionExecuterHelper
{
public:
    ActionExecuterHelper()
        : q(0)
    {}
    
    virtual ~ActionExecuterHelper()
    {
        delete q;
    }
    
    ActionExecuter* q;
};

K_GLOBAL_STATIC(ActionExecuterHelper, s_actionexecuter);

ActionExecuter::ActionExecuter(const QList< Actions::Action* >& a)
    : QObject()
    , actions(a)
    , m_valid(true)
{
    if (s_actionexecuter->q) {
        m_valid = false;
        return;
    }
    
    s_actionexecuter->q = this;
}

ActionExecuter::~ActionExecuter()
{
    delete s_actionexecuter->q;
    s_actionexecuter->q = 0;
}

bool ActionExecuter::valid() const
{
    return m_valid;
}

}
}