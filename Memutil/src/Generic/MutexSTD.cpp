#include <MemUtil/Generic/MutexSTD.hpp>

namespace MU
{
void MutexSTD::lock()
{
    m_mtx.lock();
}
void MutexSTD::unlock()
{
    m_mtx.unlock();
}
}  // namespace MU
