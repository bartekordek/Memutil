#pragma once

#include <MemUtil/IStack.hpp>

#if defined( MU_WINDOWS )

namespace MU
{
class CStackWin64 final: public CIStack
{
public:
    void fetch() override;
    void decode() override;

    bool operator==( const CStackWin64& arg ) const;
    const std::array<SLineInfo, G_MaxStackSize>& getStackLines() const override;

protected:
private:
    std::array<SLineInfo, G_MaxStackSize> m_stackFrames;
};

using CStack = CStackWin64;

}  // namespace MU
#endif  // #if defined( MU_WINDOWS )
