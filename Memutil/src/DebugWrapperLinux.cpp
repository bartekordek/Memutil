#include "DebugWrapperLinux.hpp"

#if defined( MU_LINUX )
#include <MemUtil/STL_Imports/STD_assert.hpp>
#include <MemUtil/Import_tracy.hpp>

namespace MU
{

DebugWrapperLinux::DebugWrapperLinux()
{
}

void DebugWrapperLinux::init()
{
    MU_MEASURE_SCOPE;
}

void DebugWrapperLinux::fillData( std::array<void*, G_MaxStackSize>& inOutData )
{
}

bool DebugWrapperLinux::getLineByOffset( std::uint64_t offset, std::uint64_t& inOutlineNum, char* inOutName, std::size_t inOutNameSize,
                                         std::uint64_t& outSize )
{
    return false;
}

DebugWrapperLinux::~DebugWrapperLinux()
{
}
}  // namespace MU

#endif  // #if defined( MU_LINUX )