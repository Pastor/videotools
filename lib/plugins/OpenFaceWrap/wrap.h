#pragma once
#include <Windows.h>

namespace Constants
{
    LPCTSTR PluginBasePath = TEXT("plugins.open_face.path");
}

#if defined(__cplusplus)
extern "C" {
#endif

    struct tagSESSION_POINT {
        int   x;
        int   y;
    };
    typedef struct tagSESSION_POINT Point;

    __declspec(dllexport)
        void create_wrap(const char * const modelPath);

    __declspec(dllexport)
        void process_wrap(const void *pImage, Point **p, int *nSize);

    __declspec(dllexport)
        void destroy_wrap();

#if defined(__cplusplus)
}
#endif
