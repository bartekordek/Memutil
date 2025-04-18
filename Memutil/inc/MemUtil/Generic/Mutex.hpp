#pragma once

#include <MemUtil/Generic/MutexSTD.hpp>
#include <MemUtil/Generic/MutexWin64.hpp>
#include <MemUtil/Generic/MutexS.hpp>

namespace MU
{
#if defined( _MSC_VER )
using Mutex = MutexWin64;
#else   // #if defined( _MSC_VER )
using Mutex = MutexSTD;
#endif  // #if defined( _MSC_VER )
}