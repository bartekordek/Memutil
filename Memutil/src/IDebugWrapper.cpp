#include <MemUtil/IDebugWrapper.hpp>
#include "DebugWrapperWin64.hpp"

namespace MU
{

IDebugWrapper& IDebugWrapper::getInstance()
{
#if defined(_WIN32)
    static DebugWrapperWin64 s_instance;
    return s_instance;
#endif // defined(_WIN32)
}

IDebugWrapper::IDebugWrapper()
{
}

IDebugWrapper::~IDebugWrapper()
{
}

}