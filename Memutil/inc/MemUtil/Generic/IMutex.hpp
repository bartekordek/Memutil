#pragma once

namespace MU
{

class IMutex
{
public:
    static IMutex* createDefaultMtx();
    static IMutex* createSTDMtx();
    static IMutex* createWin64Mtx();

    IMutex();
    IMutex( const IMutex& ) = delete;
    IMutex( IMutex&& ) = delete;
    IMutex& operator=( const IMutex& ) = delete;
    IMutex& operator=( IMutex&& ) = delete;

    virtual void lock() = 0;
    virtual void unlock() = 0;

    virtual ~IMutex();

protected:
private:
};

class MutexGuard final
{
public:
    MutexGuard( IMutex& inMtx );

    MutexGuard( const MutexGuard& ) = delete;
    MutexGuard( MutexGuard&& ) = delete;
    MutexGuard& operator=( const MutexGuard& ) = delete;
    MutexGuard& operator=( MutexGuard&& ) = delete;


    ~MutexGuard();

protected:
private:
    IMutex& m_mtx;
};

}  // namespace MU
