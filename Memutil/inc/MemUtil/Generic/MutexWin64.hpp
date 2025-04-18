#pragma once

#if defined( _MSC_VER )
#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/Generic/Import_windows.hpp>

namespace MU
{

class MutexWin64 final: public IMutex
{
public:
    MutexWin64();

    MutexWin64( const MutexWin64& ) = delete;
    MutexWin64( MutexWin64&& ) = delete;
    MutexWin64& operator=( const MutexWin64& ) = delete;
    MutexWin64& operator=( MutexWin64&& ) = delete;

    ~MutexWin64();

protected:
private:
    void lock() override;
    void unlock() override;

    CRITICAL_SECTION m_cs;
};

}  // namespace MU


#endif  // #if defined(_MSC_VER)