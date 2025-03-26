#pragma once

#if defined MU_EXPORT && defined MU_LINUX
#define MULib_API
#define MULib_API_POST
#define MULib_API_TEMPLATE
#elif defined MU_EXPORT && defined( MU_WINDOWS )
#define MULib_API __declspec( dllexport )
#define MULib_API_POST __cdecl
#define MULib_API_TEMPLATE
#else
#define MULib_API
#define MULib_API_POST
#define MULib_API_TEMPLATE
#endif