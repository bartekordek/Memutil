#pragma once

#include <MemUtil/Config.hpp>

#if defined( MU_WINDOWS )

#include <MemUtil/IDebugWrapper.hpp>
#include <MemUtil/Import_windows.hpp>

extern "C"
{
    typedef unsigned long( __stdcall* ___mu_t_RtlWalkFrameChain )( void**, unsigned long, unsigned long );
    extern ___mu_t_RtlWalkFrameChain ___mu_RtlWalkFrameChain;
}


namespace MU
{
class DebugWrapperWin64 final: public IDebugWrapper
{
public:
    DebugWrapperWin64();
    ~DebugWrapperWin64();

    MU_NONCOPYABLE( DebugWrapperWin64 );

protected:
private:
    void init() override;
    void fillData( std::array<void*, G_MaxStackSize>& inOutData ) override;
    bool getLineByOffset( std::uint64_t offset, std::uint64_t& inOutlineNum, char* inOutName, std::size_t inOutNameSize,
                          std::uint64_t& outSize ) override;

    ___mu_t_RtlWalkFrameChain ___mu_RtlWalkFrameChain = 0;

    IDebugClient* m_debugClient{ nullptr };
    IDebugControl* m_debugControl{ nullptr };
    IDebugSymbols* m_debugSymbols{ nullptr };
};
}  // namespace MU

#endif  // #if defined( MU_WINDOWS )