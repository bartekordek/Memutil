#pragma once

#include <MemUtil/STL_Imports/STD_functional.hpp>

namespace MU
{
class ScopeExit final
{
public:
    ScopeExit( std::function<void( void )> inFunction );
    ~ScopeExit();

    ScopeExit( const ScopeExit& ) = delete;
    ScopeExit( ScopeExit&& ) = delete;
    ScopeExit& operator=( const ScopeExit& ) = delete;
    ScopeExit& operator=( ScopeExit&& ) = delete;

protected:
private:
    std::function<void( void )> m_function;

};
}  // namespace MU