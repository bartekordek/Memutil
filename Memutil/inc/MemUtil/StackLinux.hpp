#pragma once

#include <MemUtil/IStack.hpp>

#if defined( MU_LINUX )

namespace MU
{
class CStackLinux final: public CIStack
{
public:
    void fetch() override;
    void decode() override;
    const std::array<SLineInfo, G_MaxStackSize>& getStackLines() const override;

protected:
private:
    std::array<SLineInfo, G_MaxStackSize> m_stackFrames;
};

using CStack = CStackLinux;

}  // namespace MU

#endif  // #if defined( MU_LINUX )