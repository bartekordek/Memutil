#include <MemUtil/Generic/ScopeExit.hpp>

using namespace MU;

ScopeExit::ScopeExit( std::function<void( void )> inFunction ):
    m_function( inFunction )
{
}

ScopeExit::~ScopeExit()
{
    m_function();
}
