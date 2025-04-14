#include <MemUtil/StackLinux.hpp>

#if defined( MU_LINUX )

namespace MU
{
void CStackLinux::fetch()
{
}

void CStackLinux::decode()
{
}

const std::array<SLineInfo, G_MaxStackSize>& CStackLinux::getStackLines() const
{
    return m_stackFrames;
}

}  // namespace MU

#endif // #if defined( MU_LINUX )
