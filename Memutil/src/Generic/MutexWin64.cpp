#include <MemUtil/Generic/MutexWin64.hpp>

#if defined( _MSC_VER )

namespace MU
{
MutexWin64::MutexWin64()
{
    InitializeCriticalSection( &m_cs );
}

void MutexWin64::lock()
{
    EnterCriticalSection( &m_cs );
}

void MutexWin64::unlock()
{
    LeaveCriticalSection( &m_cs );
}

MutexWin64::~MutexWin64()
{
    DeleteCriticalSection( &m_cs );
}
}  // namespace MU

#endif  // #if defined( _MSC_VER )