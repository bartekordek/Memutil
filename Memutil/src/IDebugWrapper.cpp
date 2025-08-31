#include <MemUtil/IDebugWrapper.hpp>
#include "DebugWrapperWin64.hpp"
#include "DebugWrapperLinux.hpp"

namespace MU
{

IDebugWrapper& IDebugWrapper::getInstance()
{
#if defined( _WIN32 )
    static DebugWrapperWin64 s_instance;
#else
    static DebugWrapperLinux s_instance;
#endif  // defined(_WIN32)
    return s_instance;
}

IDebugWrapper::IDebugWrapper()
{
}

IDebugWrapper::~IDebugWrapper()
{
}

}  // namespace MU