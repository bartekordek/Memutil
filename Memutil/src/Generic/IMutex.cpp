#include <MemUtil/Generic/IMutex.hpp>
#include <MemUtil/Generic/Mutex.hpp>

namespace MU
{

IMutex* IMutex::createDefaultMtx()
{
#if defined( _MSC_VER )
    return createWin64Mtx();
    // return createSTDMtx();
    // return new MutexS();
#else   // #if defined( _MSC_VER )
    return createSTDMtx();
#endif  // #if defined( _MSC_VER )
}

IMutex* IMutex::createSTDMtx()
{
    return new MutexSTD();
}

IMutex* IMutex::createWin64Mtx()
{
#if defined( _MSC_VER )
    return new MutexWin64();
#else   // #if defined( _MSC_VER )
    return nullptr;
#endif  // #if defined( _MSC_VER )
}

IMutex::IMutex()
{
}

IMutex::~IMutex()
{
}

MutexGuard::MutexGuard( IMutex* inMtx ):
    m_mtx( inMtx )
{
    m_mtx->lock();
}

MutexGuard::~MutexGuard()
{
    m_mtx->unlock();
}

}  // namespace MU
