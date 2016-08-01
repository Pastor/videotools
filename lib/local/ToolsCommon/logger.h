#pragma once
#include <memory>

class LoggerPrivate;

class Logger
{
    std::unique_ptr<LoggerPrivate> d;
public:
    explicit Logger(const char * const szFileName, bool showDate = true);
    ~Logger();

    void event() const;
    void event(int id) const;
    void printf(const char * const szFormat, ...) const;
    void printf(const wchar_t * const szFormat, ...) const;
};


