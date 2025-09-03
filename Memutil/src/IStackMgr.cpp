#include <MemUtil/IStackMgr.hpp>
#include "StackMgrWin64.hpp"

namespace MU
{
IStackMgr& IStackMgr::getInstance()
{
    static StackMgrWin64 instance;
    return instance;
}

IStackMgr::IStackMgr()
{
}

IStackMgr::~IStackMgr()
{
}
}  // namespace MU
