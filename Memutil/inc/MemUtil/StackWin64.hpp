#pragma once

#include <MemUtil/IStack.hpp>

#if defined( MU_WINDOWS )

namespace MU
{
class CStackWin64 final: public CIStack
{
public:
    CStackWin64();
    CStackWin64( const CStackWin64& arg );
    CStackWin64( CStackWin64&& arg );
    CStackWin64& operator=( const CStackWin64& arg );
    CStackWin64& operator=( CStackWin64&& arg );

    void fetch() override;
    void decode() override;

    bool operator==( const CStackWin64& arg ) const;
    const StackContents& getStackLines() const override;

    ~CStackWin64();

protected:
private:
    StackContents m_stackFrames;
    std::array<void*, G_MaxStackSize> m_data;
};

using CStack = CStackWin64;

}  // namespace MU
#endif  // #if defined( MU_WINDOWS )
