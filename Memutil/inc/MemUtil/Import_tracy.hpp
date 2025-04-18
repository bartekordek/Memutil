#pragma once

#define MU_USE_TRACY 1

#include "MemUtil/Generic/DisableWarnings.hpp"

#if MU_USE_TRACY
#include "tracy/Tracy.hpp"
#include "tracy/TracyC.h"
#endif  // #if MU_USE_TRACY

#if defined( _MSC_VER )
#pragma warning( pop )
#endif  // #if defined( _MSC_VER )

#if MU_USE_TRACY
#define MU_SET_THREAD_NAME TracyCSetThreadName
#define MU_MEASURE_SCOPE ZoneScoped
#define MU_MEASURE_SUBSCOPE ZoneNamedN
#else
#define MU_MEASURE_SCOPE
#define MU_MEASURE_SUBSCOPE( VAL1, VAL2, VAL3 )
#define MU_SET_THREAD_NAME( VAL )
#endif