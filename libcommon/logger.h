#pragma once
#include <memory>
#include <atomic>
#include <mutex>
#include <cstdio>
#include <cstdlib>

class LoggerPrivate;

class Logger
{
    std::unique_ptr<LoggerPrivate> d;
public:
    explicit Logger(const char * const szFileName, bool showDate = true);
    ~Logger();

    void event();
    void event(int id);
    void printf(const char * const szFormat, ...);
    void printf(const wchar_t * const szFormat, ...);
};


