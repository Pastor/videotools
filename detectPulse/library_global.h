#ifndef LIBRARY_GLOBAL_H
#define LIBRARY_GLOBAL_H

#define LIBRARY_EXPORT __declspec(dllexport)

#ifndef LIBRARY_EXTERN_C
#  define LIBRARY_EXTERN_C extern "C"
#endif

#ifndef LIBRARYAPI
#  define LIBRARYAPI(RetType) LIBRARY_EXTERN_C LIBRARY_EXPORT RetType
#endif

#if defined(Q_OS_WIN32) && defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif

#endif // LIBRARY_GLOBAL_H

