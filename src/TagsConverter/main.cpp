#include <Windows.h>
#include <commctrl.h>
#include <strsafe.h>
#include <Shlwapi.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <logger.h>
#include <xstring.h>
#include <boost/filesystem.hpp>

#include "constants.h"
#include "resource.h"

#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Rpcrt4.lib")
#pragma comment(lib, "Winmm.lib")


#pragma warning(disable: 4996)

#define FIX_FPS           30

#define WM_PROCESS_LINE       WM_USER + 100
#define WM_PROCESS_COMPLETE   WM_USER + 101
#define WM_PROCESS_ERROR      WM_USER + 102

#define OBJECT_MINSIZE   80
#include <properties.h>
#include <sqlite3.h>
#include <sqlite.hpp>

static Logger *gLogger = nullptr;
static Properties *gProp = nullptr;
static std::atomic_bool is_running;
static std::atomic_bool is_processing;

namespace fs = boost::filesystem;

INT_PTR CALLBACK
MainDialog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

static __int64
file_size(LPCSTR lpcstrFileName)
{
    WIN32_FILE_ATTRIBUTE_DATA fad;
    if (!GetFileAttributesExA(lpcstrFileName, GetFileExInfoStandard, &fad))
        return -1;
    LARGE_INTEGER size;
    size.HighPart = fad.nFileSizeHigh;
    size.LowPart = fad.nFileSizeLow;
    return size.QuadPart;
}

inline std::vector<std::string> &
split_string(const std::string &s, char delim, std::vector<std::string> &elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(std::trim(item));
    }
    return elems;
}

static xstring
CreateGuid()
{
#if defined(UNICODE)
#define STR    RPC_WSTR
#else
#define STR    RPC_CSTR
#endif
    UUID    uuid;
    xstring result;
    if (UuidCreate(&uuid) == RPC_S_OK) {
        LPTSTR lpResult;
        UuidToString(&uuid, reinterpret_cast<STR *>(&lpResult));
        result = xstring(lpResult);
        RpcStringFree(reinterpret_cast<STR *>(&lpResult));
    }
    return result;
#undef STR
}


int WINAPI
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    HWND                    hMainWnd;
    MSG                     msg;

    sqlite3_initialize();
    gLogger = new Logger("tagsconverter.log");
    gProp = new Properties();
    gProp->load("settings.properties");
    gLogger->printf(TEXT("Запуск"));
    InitCommonControls();
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    hMainWnd = CreateDialogParam(hInstance, TEXT("MAINDIALOG"), nullptr, MainDialog, reinterpret_cast<LPARAM>(hInstance));
    ShowWindow(hMainWnd, SW_SHOW);
    is_running = true;
    do {
        if (PeekMessage(&msg, hMainWnd, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (msg.message != WM_QUIT && is_running);
    DestroyWindow(hMainWnd);
    gProp->save("settings.properties");
    delete gProp;
    gLogger->printf(TEXT("Остановка"));
    delete gLogger;
    return EXIT_SUCCESS;
}


static void
__WindowPrintf(HWND hWnd, LPCTSTR lpstrFormat, ...)
{
    va_list args;
    int     ret;
    LPTSTR  lpPrepared;
    auto    len = std::wcslen(lpstrFormat) * 4;

    lpPrepared = static_cast<LPWSTR>(std::malloc(len * sizeof(WCHAR)));
    va_start(args, lpstrFormat);
    ret = vswprintf(lpPrepared, len, lpstrFormat, args);
    va_end(args);
    lpPrepared[ret] = static_cast<TCHAR>(0);
    SetWindowText(hWnd, lpPrepared);
    std::free(lpPrepared);
}

struct ProcessParam
{
    HWND         hwnd;
    CHAR         szFileTemplate[MAX_PATH * 2];
    CHAR         szFileName[MAX_PATH * 2];
    std::string  table_name;
    std::string  database_name;
    __int64 size;
};

struct ProcessLine
{
    __int64 process;
    __int64 line;
    DWORD dwMiddleTime;
    DWORD dwProcessTime;
};

static DWORD
__ProcessFile(LPVOID pParam)
{
    auto p = static_cast<ProcessParam *>(pParam);
    DWORD dwAllTime = 0;
    ProcessLine processLine;

    processLine.line = 0;
    processLine.process = 0;
    auto startProcess = timeGetTime();
    std::ifstream file(p->szFileName);
    std::string line;
    std::vector<std::string> parts;

    if (p->database_name.size() == 0) {
        p->database_name = std::toString(CreateGuid());
        gProp->setString(Constants::DatabaseName, p->database_name.c_str());
    }
    Sqlite::Database database(p->database_name + std::string(".db"), SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE);

    std::getline(file, line);
    split_string(line, ';', parts);

    auto i = 0;
    auto query{ std::string("CREATE TABLE IF NOT EXISTS '") + p->table_name + std::string("' (\r\n") };
    auto insert_start{ std::string("INSERT INTO '") + p->table_name + std::string("'( ") };
    auto insert_end{ std::string("VALUES(") };
    for (auto it = parts.begin(); it != parts.end(); ++it) {
        auto item = (*it);
        std::transform(item.begin(), item.end(), item.begin(), ::tolower);
        std::trim(item);
        if (i > 0) {
            query += std::string(", \r\n");
            insert_start += std::string(", ");
            insert_end += std::string(", ");
        }
        if (item == std::string("frame")) {
            query += std::string("  'frame' TEXT NOT NULL");
            insert_start += std::string("'frame'");
        } else {
            query += std::string("  '") + item + std::string("' TEXT DEFAULT NULL");
            insert_start += std::string("'") + item + std::string("'");
        }
        insert_end += std::string("?");
        ++i;
    }
    query += std::string(")");
    insert_start += std::string(") ");
    insert_end += std::string(") ");
    auto insert_query{ insert_start + insert_end };
    try {
        database.exec(query);
    } catch (Sqlite::Exception ex) {
        SendMessage(p->hwnd, WM_PROCESS_ERROR, 0, reinterpret_cast<LPARAM>(ex.what()));
        is_processing = false;
        return -1;
    }
    is_processing = true;
    Sqlite::Statement insert{ database, insert_query.c_str() };
    database.begin_transaction();
    try {
        while (std::getline(file, line) && is_processing) {
            auto start = timeGetTime();
            processLine.line++;
            processLine.process += line.size() + 2;

            parts.clear();
            split_string(line, ';', parts);
            for (auto it = parts.begin(); it != parts.end(); ++it) {
                auto &item = (*it);
                insert.bind_text(item.c_str());
            }
            insert.execute();
            dwAllTime += (timeGetTime() - start);
            processLine.dwMiddleTime = processLine.process > 0 ? (dwAllTime / processLine.process) : 0;
            processLine.dwProcessTime = timeGetTime() - startProcess;
            SendMessage(p->hwnd, WM_PROCESS_LINE, 0, reinterpret_cast<LPARAM>(&processLine));
        }
        database.commit();
    } catch (Sqlite::Exception ex) {
        SendMessage(p->hwnd, WM_PROCESS_ERROR, 0, reinterpret_cast<LPARAM>(ex.what()));
    }

    try {
        database.exec(std::string("CREATE UNIQUE INDEX IF NOT EXISTS 'frame_index' ON '") + p->table_name + std::string("'('frame')"));
    } catch (Sqlite::Exception ex) {
        SendMessage(p->hwnd, WM_PROCESS_ERROR, 0, reinterpret_cast<LPARAM>(ex.what()));
    }
    is_processing = false;
    SendMessage(p->hwnd, WM_PROCESS_COMPLETE, 0, 0);
    return 0;
}

INT_PTR CALLBACK
MainDialog(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HINSTANCE hInstance = nullptr;
    static HFONT hFont = nullptr;
    static auto  cDefColor = RGB(240, 240, 240);
    static auto  cStatusColor = RGB(0, 0, 0);
    static auto  cErrorColor = RGB(178, 34, 34);
    static HBRUSH hDefBrush = nullptr;

    static HBITMAP hBitmap = nullptr;

    static HMENU hMainMenu = nullptr;
    static HWND  hProgress = nullptr;
    static HWND  hError = nullptr;
    static HWND  hStartProcess = nullptr;
    static HWND  hStopProcess = nullptr;
    static HWND  hCurrentLine = nullptr;
    static HWND  hProcessPercent = nullptr;

    static HANDLE hProcessThread = nullptr;
    static DWORD  dwProcessThreadId;
    static ProcessParam param;

    switch (uMsg) {
    case WM_INITDIALOG:
    {
        hInstance = reinterpret_cast<HINSTANCE>(lParam);

        hProgress = GetDlgItem(hWnd, IDC_PROCESS_PROGRESS);
        hStartProcess = GetDlgItem(hWnd, IDC_START_PROCESS);
        hStopProcess = GetDlgItem(hWnd, IDC_STOP_PROCESS);
        hCurrentLine = GetDlgItem(hWnd, IDC_INFO_CURRENT_LINE);
        hError = GetDlgItem(hWnd, IDC_ERROR_TEXT);
        hProcessPercent = GetDlgItem(hWnd, IDC_PROCESS_PERCENT);
        hDefBrush = CreateSolidBrush(cDefColor);
        hFont = CreateFont(12, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
            OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, TEXT("Lucida Console"));
        hMainMenu = LoadMenu(hInstance, MAKEINTRESOURCE(IDR_MAINMENU));
        SetMenu(hWnd, hMainMenu);
        SetWindowText(hProcessPercent, TEXT(""));
        SetWindowText(hError, TEXT(""));

        SendMessage(hProcessPercent, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), static_cast<LPARAM>(0));
        {
            auto name = gProp->getWString(Constants::ApplicationName);
            if (name.size() > 0) {
                SetWindowText(hWnd, name.c_str());
            }
            auto database_name = gProp->getString(Constants::DatabaseName);
            if (database_name.size() > 0) {
                param.database_name = database_name;
            }
        }
        param.hwnd = hWnd;
        param.size = 0;
        return TRUE;
    }
    case WM_NOTIFY:
    {
        auto wCtrlId = LOWORD(wParam);
        auto lHdr = reinterpret_cast<LPNMHDR>(lParam);
        break;
    }
    case WM_COMMAND:
    {
        auto wNotifyId = HIWORD(wParam);
        auto wCtrlId = LOWORD(wParam);

        switch (wCtrlId) {
        case ID_FILE_QUIT:
        {
            SendMessage(hWnd, WM_CLOSE, 0, 0);
            break;
        }
        case ID_FILE_OPEN:
        {
            OPENFILENAMEA ofn;
            RtlSecureZeroMemory(&ofn, sizeof(ofn));
            RtlSecureZeroMemory(param.szFileName, sizeof(param.szFileName));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hWnd;
            ofn.lpstrFile = param.szFileName;
            ofn.nMaxFile = sizeof(param.szFileName);
            ofn.lpstrFilter = "Все\0*.*\0Текстовый файл\0*.txt\0Файл CSV\0*.csv\0Файл LDOTS\0*.ldots\0";
            ofn.nFilterIndex = 4;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;
            if (GetOpenFileNameA(&ofn) == TRUE) {
                StringCchCopyA(param.szFileTemplate, sizeof(param.szFileTemplate), param.szFileName);
                PathRemoveExtensionA(param.szFileTemplate);
                param.size = file_size(param.szFileName);
                param.table_name = fs::basename(std::string(param.szFileTemplate));
                SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("Запуск"));
                EnableWindow(hStartProcess, TRUE);
            } else {
                EnableWindow(hStartProcess, FALSE);
            }
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            break;
        }
        case IDC_START_PROCESS:
        {
            SetWindowText(hError, TEXT(""));
            SendMessage(hProgress, PBM_SETSTEP, static_cast<WPARAM>(1), 0);
            SendMessage(hProgress, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
            hProcessThread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(__ProcessFile), &param, 0, &dwProcessThreadId);
            EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
            EnableWindow(hStartProcess, FALSE);
            break;
        }
        case IDC_STOP_PROCESS:
        {
            is_processing = false;
            break;
        }

        }
        break;
    }
    case WM_PROCESS_LINE:
    {
        auto pl = reinterpret_cast<ProcessLine *>(lParam);
        auto percent = (pl->process * 100) / param.size;
        auto tail = param.size - pl->process;

        __WindowPrintf(hCurrentLine, TEXT("%d"), pl->line);
        __WindowPrintf(hProcessPercent, TEXT("%d%%"), percent);
        {
            auto total = tail * pl->dwMiddleTime;
            auto sec = total / 1000 % 60;
            auto minutes = total / 1000 / 60 % 60;
            auto hours = total / 1000 / 60 / 60 % 24;
            auto days = total / 1000 / 60 / 60 / 24 % 7;
            __WindowPrintf(GetDlgItem(hWnd, IDC_INFO_TIME), TEXT("%02d:%02d:%02d"), hours, minutes, sec);
        }
        {
            auto sec = pl->dwProcessTime / 1000 % 60;
            auto minutes = pl->dwProcessTime / 1000 / 60 % 60;
            auto hours = pl->dwProcessTime / 1000 / 60 / 60 % 24;
            auto days = pl->dwProcessTime / 1000 / 60 / 60 / 24 % 7;
            __WindowPrintf(GetDlgItem(hWnd, IDC_INFO_TIME_COMPLETE), TEXT("%02d:%02d:%02d"), hours, minutes, sec);
        }
        SendMessage(hProgress, PBM_SETPOS, percent, 0);
        break;
    }
    case WM_PROCESS_ERROR:
    {
        auto error = reinterpret_cast<LPCSTR>(lParam);
        SetWindowTextA(hError, error);
        UpdateWindow(hError);

    }
    case WM_PROCESS_COMPLETE:
    {
        if (hProcessThread != nullptr)
            CloseHandle(hProcessThread);
        hProcessThread = nullptr;
        dwProcessThreadId = 0;
        EnableWindow(hStopProcess, hProcessThread == nullptr ? FALSE : TRUE);
        SendMessage(hProgress, PBM_SETPOS, 0, 0);
        SetWindowText(hProcessPercent, TEXT(""));
        SetWindowText(GetDlgItem(hWnd, IDC_START_PROCESS), TEXT("Запуск"));
        break;
    }
    case WM_CTLCOLORSTATIC:
    {
        auto hdc = reinterpret_cast<HDC>(wParam);
        if (reinterpret_cast<HWND>(lParam) == nullptr) {
            SetTextColor(hdc, cStatusColor);
            SetBkColor(hdc, cDefColor);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(hDefBrush);
        } else if (reinterpret_cast<HWND>(lParam) == hError) {
            SetTextColor(hdc, cErrorColor);
            SetBkColor(hdc, cDefColor);
            SetBkMode(hdc, TRANSPARENT);
            return reinterpret_cast<INT_PTR>(hDefBrush);
        }
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    case WM_CLOSE:
    {
        if (hProcessThread != nullptr) {
            is_processing = false;
            Sleep(1000);
            CloseHandle(hProcessThread);
            hProcessThread = nullptr;
        }
        EndDialog(hWnd, lParam);
        PostQuitMessage(0);
        is_running = false;
        return TRUE;
    }
    }
    return FALSE;
}
