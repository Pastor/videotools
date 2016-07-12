#pragma once
#include <Windows.h>
#include <videoplugin.h>

namespace Constants
{
    LPCTSTR PluginBasePath = TEXT("plugins.open_face.path");
}

#if defined(__cplusplus)
extern "C" {
#endif
    __declspec(dllexport)
        INT LoadPlugin(VideoPlugin *pluginContext);

    __declspec(dllexport)
        INT FreePlugin(VideoPlugin *pluginContext);

#if defined(__cplusplus)
}
#endif
