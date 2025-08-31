#pragma once

#include <MemUtil/Config.hpp>

#if defined( MU_LINUX )

#include <MemUtil/IDebugWrapper.hpp>

namespace MU
{
class DebugWrapperLinux final: public IDebugWrapper
{
public:
    DebugWrapperLinux();
    ~DebugWrapperLinux();

    MU_NONCOPYABLE( DebugWrapperLinux )

protected:
private:
    void init() override;
    void fillData( std::array<void*, G_MaxStackSize>& inOutData ) override;
    bool getLineByOffset( std::uint64_t offset, std::uint64_t& inOutlineNum, char* inOutName, std::size_t inOutNameSize,
                          std::uint64_t& outSize ) override;
};
}  // namespace MU

#endif  // #if defined( MU_WINDOWS )