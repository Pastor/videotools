#include <Windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <shlobj.h>
#include <Dbt.h>
#include <shlobj.h>
#include <Shlwapi.h>

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")

#include "system_helper.h"


std::string 
absFilePath(const std::string& fileName)
{
    CHAR        szPath[MAX_PATH * 4];

    GetModuleFileNameA(GetModuleHandle(nullptr), szPath, sizeof(szPath));
    PathRemoveFileSpecA(szPath);
    PathCombineA(szPath, szPath, fileName.c_str());
    return szPath;
}