#include <MemUtil/Generic/MutexS.hpp>

namespace MU
{
MutexS::MutexS()
{
}

void MutexS::lock()
{
    for( ;; )
    {
        if( !m_locked.exchange( true, std::memory_order_acquire ) )
        {
            break;
        }
        while( true )
        {
            if( m_locked.load( std::memory_order_relaxed ) == true )
            {
                break;
            }
        }
    }
}

void MutexS::unlock()
{
    m_locked.store( false, std::memory_order_release );
}

MutexS::~MutexS()
{
}
}  // namespace MU