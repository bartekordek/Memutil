#pragma once

#include <MemUtil/Config.hpp>
#include <MemUtil/Generic/NonCopyable.hpp>
#include <MemUtil/STL_Imports/STD_array.hpp>
#include <MemUtil/STL_Imports/STD_cstdint.hpp>

namespace MU
{
class IDebugWrapper
{
public:
    static IDebugWrapper& getInstance();
    virtual void init() = 0;
    virtual void fillData( std::array<void*, G_MaxStackSize>& inOutData ) = 0;
    virtual bool getLineByOffset( std::uint64_t offset, std::uint64_t& inOutlineNum, char* inOutName, std::size_t inOutNameSize,
                                  std::uint64_t& outSize ) = 0;

    MU_NONCOPYABLE( IDebugWrapper )

protected:
    IDebugWrapper();
    virtual ~IDebugWrapper();

private:
};
}  // namespace MU