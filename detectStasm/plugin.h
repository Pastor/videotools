#include <Windows.h>
#include <videoplugin.h>

#if defined(__cplusplus)
extern "C" {
#endif
    __declspec(dllexport)
        INT LoadPlugin(VideoPlugin *pluginContext);

    __declspec(dllexport)
        INT FreePlugin(VideoPlugin *pluginContext);

    __declspec(dllexport)
        INT ProcessFrame(VideoPluginFrameContext *frameContext);

    __declspec(dllexport)
        INT StartProcess(VideoPluginStartContext *startContext);

    __declspec(dllexport)
        INT StopProcess(VideoPluginStartContext *startContext);
#if defined(__cplusplus)
}
#endif

#define ALLOCATED_LANDMARKS  200

/** External functions */
#if defined(__cplusplus)
extern "C" {
#endif
	__declspec(dllexport)
		INT __CreateProcess(const char *directory);

	__declspec(dllexport)
		INT __FrameProcess(IplImage *iFrame, float landmarks[ALLOCATED_LANDMARKS], int *iLandmarks);
#if defined(__cplusplus)
}
#endif



