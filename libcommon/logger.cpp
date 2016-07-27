#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS
#include <memory>
#include <atomic>
#include <mutex>
#include <cstdio>
#include <cstdlib>
#include <strsafe.h>
#include <Windows.h>
#include <mutex>
#include "logger.h"

static void
__SafeClose(FILE **fd)
{
    __try {
        if (*fd != nullptr)
            fclose(*fd);
        *fd = nullptr;
    } __except (EXCEPTION_EXECUTE_HANDLER) {

    }
}

class LoggerPrivate
{
public:
    LoggerPrivate()
        : allocated(0), offset(0), event(0), szBuffer(nullptr), fd(nullptr), showDate(true)
    {}
    bool open(const char * const szFileName)
    {
        fd = fopen(szFileName, "a");
        return fd != nullptr;
    }
    bool close()
    {
        __SafeClose(&fd);
        return true;
    }
    bool free()
    {
        if (allocated > 0 && szBuffer != nullptr)
            std::free(szBuffer);
        szBuffer = nullptr;
        
        allocated = 0;
        return true;
    }
    bool allocate(std::size_t size)
    {
        if (offset + size >= allocated) {
            allocated += size + 20;
            if (szBuffer == nullptr) {
                szBuffer = static_cast<char *>(std::malloc(allocated));
            } else {
                szBuffer = static_cast<char *>(std::realloc(szBuffer, allocated));
            }
        }
        return true;
    }
    bool clear()
    {
        std::memset(szBuffer, 0, allocated);
        offset = 0;
        return true;
    }
    bool write()
    {
        return write(szBuffer, offset) == offset;
    }
    bool flush()
    {
        if (write())
            return clear();
        return false;
    }
    void add(const char * const szData)
    {
        std::size_t len = std::strlen(szData);
        
        allocate(len);
        memcpy(szBuffer + offset, szData, len);
        offset += len;
        flush();
    }
    void addDate()
    {
        SYSTEMTIME s;
        char       szBuffer[256];

        GetSystemTime(&s);
        if (showDate) {
            StringCchPrintfA(szBuffer, sizeof(szBuffer), "%02d.%02d.%04d %02d:%02d:%02d.%04d [%06d] ",
                s.wDay,
                s.wMonth,
                s.wYear,

                s.wHour,
                s.wMinute,
                s.wSecond,
                s.wMilliseconds,
                event);
        } else {
            StringCchPrintfA(szBuffer, sizeof(szBuffer), "%02d:%02d:%02d.%04d [%06d] ",
                s.wHour,
                s.wMinute,
                s.wSecond,
                s.wMilliseconds,
                event);
        }        
        clear();
        add(szBuffer);
        flush();
    }

    std::size_t write(void *pBuffer, std::size_t size)
    {
        std::size_t ret;

        ret = fwrite(pBuffer, 1, size, fd);
        if (ret > 0 && ret == size)
            fflush(fd);
        return ret;
    }

    ~LoggerPrivate()
    {
        close();
        free();
    }


    std::size_t      allocated;
    std::size_t      offset;
    std::size_t      event;
    char            *szBuffer;
    std::mutex       mutex;
    FILE            *fd;
    bool             showDate;
};

Logger::Logger(const char* const szFileName, bool showDate)
    : d(new LoggerPrivate)
{
    d->showDate = showDate;
    d->open(szFileName);
}

Logger::~Logger()
{
}

void
Logger::event()
{
    d->event++;
}

void 
Logger::event(int id)
{
    d->event = id;
}

void 
Logger::printf(const char* const szFormat, ...)
{
    std::lock_guard<std::mutex>  locker(d->mutex);
    std::size_t len = std::strlen(szFormat);
    va_list args;

    d->addDate();
    va_start(args, szFormat);
    d->allocate(len * 4);
    vsprintf(d->szBuffer, szFormat, args);
    va_end(args);
    d->add("\r\n");
}

void 
Logger::printf(const wchar_t* const szFormat, ...)
{
    std::lock_guard<std::mutex>  locker(d->mutex);
    std::size_t len = std::wcslen(szFormat);
    LPWSTR lpPrepared = nullptr;
    int ret;
    va_list args;

    d->addDate();
    d->allocate(len * 4);
    lpPrepared = static_cast<LPWSTR>(std::malloc(d->allocated * sizeof(WCHAR)));
    va_start(args, szFormat);
    ret = vswprintf(lpPrepared, d->allocated, szFormat, args);
    va_end(args);
    d->offset = WideCharToMultiByte(CP_UTF8, 0, lpPrepared, ret, d->szBuffer, d->allocated, nullptr, nullptr);
    d->add("\r\n");
    std::free(lpPrepared);
}