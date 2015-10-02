#include <shlobj.h>
#include <Shlwapi.h>
#include <strsafe.h>
#include "logger.h"
#include "videoplugin.h"

void 
LoadPlugins(LPCTSTR lpstrDirectory, Logger *logger, Properties *prop, VideoPluginList &list)
{
    WIN32_FIND_DATA    wfd;
    HANDLE             hFind;
    LPTSTR             pPrepared;
    TCHAR              szPath[MAX_PATH * 2];
    

    FreePlugins(logger, list);
    GetModuleFileName(GetModuleHandle(nullptr), szPath, sizeof(szPath));
    pPrepared = static_cast<LPTSTR>(std::malloc(sizeof(szPath) + std::wcslen(lpstrDirectory) + 256));
    PathRemoveFileSpec(szPath);
    PathCombine(pPrepared, szPath, lpstrDirectory);
    PathCombine(pPrepared, pPrepared, TEXT("*.dll"));
    RtlSecureZeroMemory(&wfd, sizeof(wfd));
    hFind = FindFirstFile(pPrepared, &wfd);
    if (hFind != nullptr && hFind != INVALID_HANDLE_VALUE) {
        std::size_t        allocated = 0;
        LPTSTR             pFileName = nullptr;
        auto               lPreparedSize = std::wcslen(pPrepared) * sizeof(TCHAR);

        allocated = (wcslen(pPrepared) + 100) * sizeof(TCHAR);
        pFileName = static_cast<LPTSTR>(std::malloc(allocated));
        PathRemoveFileSpec(pPrepared);
        do {
            if (allocated < lPreparedSize + std::wcslen(wfd.cFileName)) {
                allocated += std::wcslen(wfd.cFileName) + 10;
                pFileName = static_cast<LPTSTR>(std::realloc(pFileName, allocated));
            }
            PathCombine(pFileName, pPrepared, wfd.cFileName);
            logger->printf(TEXT("Загружаем %ls"), pFileName);
            auto hLibrary = LoadLibrary(pFileName);
            if (hLibrary != nullptr) {
                auto lp = reinterpret_cast<pfnLoadPlugin>(GetProcAddress(hLibrary, "LoadPlugin"));
                auto fp = reinterpret_cast<pfnFreePlugin>(GetProcAddress(hLibrary, "FreePlugin"));
                auto pf = reinterpret_cast<pfnProcessFrame>(GetProcAddress(hLibrary, "ProcessFrame"));
                if (lp != nullptr && fp != nullptr && pf != nullptr) {
                    auto p = static_cast<VideoPlugin *>(std::malloc(sizeof(VideoPlugin)));
                    RtlSecureZeroMemory(p, sizeof(VideoPlugin));
                    p->isActive = TRUE;
                    p->hLibrary = hLibrary;
                    p->pLoad = lp;
                    p->pFree = fp;
                    p->pProcessFrame = pf;
                    p->pStartProcess = reinterpret_cast<pfnStartProcess>(GetProcAddress(hLibrary, "StartProcess"));
                    p->pStopProcess = reinterpret_cast<pfnStopProcess>(GetProcAddress(hLibrary, "StopProcess"));
                    p->prop = prop;
                    RtlSecureZeroMemory(p->szPluginPath, sizeof(p->szPluginPath));
                    StringCchCopy(p->szPluginPath, sizeof(p->szPluginPath), pPrepared);
                    (*lp)(p);
                    list.push_back(p);
                    logger->printf(TEXT("Загружен плагин %ls версии %02d.%02d"), p->lpstrPluginName, p->wVersionMajor, p->wVersionMinor);
                }
            }
        } while (FindNextFile(hFind, &wfd));
        FindClose(hFind);
    } else {
        auto dwError = GetLastError();
        __asm nop;
    }
    std::free(pPrepared);
}

void 
FreePlugins(Logger *logger, VideoPluginList &list)
{
    if (logger != nullptr)
        logger->printf(TEXT("Выгрузка плагинов"));
    for (auto it = list.begin(); it != list.end(); ++it) {
        auto pFree = (*it)->pFree;
        if (pFree != nullptr)
            (*pFree)((*it));
        if ((*it)->hLibrary != nullptr)
            FreeLibrary((*it)->hLibrary);
        std::free((*it));
    }
    list.clear();
}

