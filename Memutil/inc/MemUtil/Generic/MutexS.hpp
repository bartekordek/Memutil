#pragma once

#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/STL_Imports/STD_atomic.hpp>

namespace MU
{

class MutexS final: public IMutex
{
public:
    MutexS();

    MutexS( const MutexS& ) = delete;
    MutexS( MutexS&& ) = delete;
    MutexS& operator=( const MutexS& ) = delete;
    MutexS& operator=( MutexS&& ) = delete;

    ~MutexS();

protected:
private:
    void lock() override;
    void unlock() override;

    std::atomic_bool m_locked{ false };
};

}  // namespace MU