#pragma once

#include <MemUtil/STL_Imports/STD_cstddef.hpp>
#include <MemUtil/STL_Imports/STD_cstdint.hpp>

namespace MU
{
constexpr std::int32_t G_MaxStackSize{ 16 };
constexpr std::int32_t G_PointerOffset{ 4 };
constexpr std::int32_t G_DataSizePlusOffset{ G_MaxStackSize + G_PointerOffset };
}

#if defined( _MSC_VER )
#define MU_WINDOWS
#else
#define MU_LINUX
#endif  // #if defined( _MSC_VER )