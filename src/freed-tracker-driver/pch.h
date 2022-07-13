#ifndef PCH_H
#define PCH_H

#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "framework.h"

#if defined(_WIN32)
#define OPENVRDRIVER_DLL_EXPORT extern "C" __declspec( dllexport )
#define OPENVRDRIVER_DLL_IMPORT extern "C" __declspec( dllimport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define OPENVRDRIVER_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#define OPENVRDRIVER_DLL_IMPORT extern "C" 
#else
#error "Unsupported Platform"
#endif

#endif //PCH_H
