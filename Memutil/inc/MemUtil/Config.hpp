#pragma once

#include <MemUtil/STL_Imports/STD_cstddef.hpp>

namespace MU
{
constexpr std::size_t G_MaxStackSize{ 16u };
}

#if defined( _MSC_VER )
#define MU_WINDOWS
#else
#define MU_LINUX
#endif  // #if defined( _MSC_VER )