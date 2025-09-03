#pragma once

#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/STL_Imports/STD_mutex.hpp>

namespace MU
{

class MutexSTD final: public IMutex
{
public:
    MutexSTD() = default;

    MutexSTD( const MutexSTD& ) = delete;
    MutexSTD( MutexSTD&& ) = delete;
    MutexSTD& operator=( const MutexSTD& ) = delete;
    MutexSTD& operator=( MutexSTD&& ) = delete;


    ~MutexSTD() = default;

protected:
private:
    void lock() override;
    void unlock() override;

    std::mutex m_mtx;

};

}  // namespace MU
