#include <Windows.h>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <atomic>
#include <mutex>
#include "xstring.h"
#include "logger.h"
#include "properties.h"

namespace Internal {
    class PropertiesPrivate final {
    public:
        explicit PropertiesPrivate()
            : hHeap(HeapCreate(HEAP_NO_SERIALIZE, 0, 0))
        {}
        ~PropertiesPrivate()
        {
            HeapDestroy(hHeap);
        }

        int    count() const;

        bool   add(const char * const szKey, const char *    const szValue);
        bool   add(const char * const szKey, const wchar_t * const szValue);
        bool   update(const char * const szKey, const char* const szValue);
        bool   update(const char * const szKey, const wchar_t* const szValue);
        bool   remove(const char * const szKey);
        std::string  getString(const char * const szKey);
        std::wstring getWString(const char * const szKey);
        void clear();
    private:
        wchar_t *alloc(const char * const szBuffer) const;
        char    *alloc(const wchar_t * const szBuffer) const;
        char    *toString(const wchar_t * const szBuffer) const;
        wchar_t *toString(const char * const szBuffer) const;
        void   free(LPVOID pMemory) const;

        HANDLE        hHeap;
        std::mutex    mutex;
        std::unordered_map<std::string, std::string> properties;
        friend class Properties;
    };

    int PropertiesPrivate::count() const
    {
        return properties.size();
    }

    bool PropertiesPrivate::add(const char * const szKey, const char * const szValue)
    {
        properties[szKey] = szValue;
        return true;
    }

    bool PropertiesPrivate::add(const char * const szKey, const wchar_t * const szValue)
    {
        auto szBuffer = toString(szValue);
        auto ret = add(szKey, szBuffer);
        free(szBuffer);
        return ret;
    }

    bool PropertiesPrivate::update(const char * const szKey, const char * const szValue)
    {
        properties[szKey] = szValue;
        return true;
    }

    bool PropertiesPrivate::update(const char * const szKey, const wchar_t * const szValue)
    {
        auto szBuffer = toString(szValue);
        auto ret = update(szKey, szBuffer);
        free(szBuffer);
        return ret;
    }

    bool PropertiesPrivate::remove(const char * const szKey)
    {
        properties.erase(properties.find(szKey));
        return true;
    }

    char    *PropertiesPrivate::toString(const wchar_t * const szValue) const
    {
        char *szBuffer;
        int   ret;

        szBuffer = alloc(szValue);
        ret = WideCharToMultiByte(
            CP_UTF8,
            0,
            szValue,
            std::wcslen(szValue),
            szBuffer,
            std::wcslen(szValue) * sizeof(wchar_t), nullptr, nullptr
            );
        szBuffer[ret] = 0;
        return szBuffer;
    }

    wchar_t *PropertiesPrivate::toString(const char * const szValue) const
    {
        int ret;
        wchar_t *szBuffer;

        szBuffer = alloc(szValue);
        ret = MultiByteToWideChar(
            CP_UTF8, 
            0, 
            szValue, 
            std::strlen(szValue), 
            szBuffer, 
            std::strlen(szValue) * sizeof(wchar_t));
        szBuffer[ret] = static_cast<wchar_t>(0);
        return szBuffer;
    }

    std::string PropertiesPrivate::getString(const char * const szKey) {
        return properties[szKey];
    }

    std::wstring PropertiesPrivate::getWString(const char * const szKey)
    {
        std::wstring result;
        std::string  get;
        wchar_t     *ret;

        get = getString(szKey);
        ret = toString(get.c_str());
        result = ret;
        free(ret);
        return result;
    }

    void PropertiesPrivate::clear()
    {
        properties.clear();
    }

    void PropertiesPrivate::free(LPVOID pMemory) const
    {
        if (pMemory != nullptr)
            HeapFree(hHeap, HEAP_NO_SERIALIZE, pMemory);
    }

    wchar_t * PropertiesPrivate::alloc(const char * const szBuffer) const
    {
        return static_cast<wchar_t *>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, (std::strlen(szBuffer) + 100) * sizeof(wchar_t)));
    }

    char * PropertiesPrivate::alloc(const wchar_t * const szBuffer) const
    {
        return static_cast<char *>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, (std::wcslen(szBuffer) + 100) * sizeof(wchar_t)));
    }
}
using namespace Internal;

Properties::Properties()
    : d(new PropertiesPrivate())
{}

std::string 
Properties::getString(const char* const szKey, const char* const szDefault) const
{
    std::lock_guard<std::mutex> locker(d->mutex);
    auto ret = d->properties[szKey];
    return ret.size() > 0 ? ret : std::string(szDefault == nullptr ? "" : szDefault);
}

std::wstring 
Properties::getWString(const char* const szKey, const wchar_t* const szDefault) const
{
    auto ret = d->getWString(szKey);
    return ret.size() > 0 ? ret : std::wstring(szDefault == nullptr ? L"" : szDefault);
}

void 
Properties::setString(const char* const szKey, const char* const szValue) const
{
    d->properties[szKey] = szValue;
}

void 
Properties::setString(const char* const szKey, const wchar_t* const szValue) const
{
    if (!d->add(szKey, szValue))
        d->update(szKey, szValue);
}

int 
Properties::getInteger(const char* const szKey, int iDefault) const
{
    std::string::size_type sz;
    auto ret = getString(szKey);
    return ret.size() > 0 ? std::stoi(ret, &sz) : iDefault;
}

float 
Properties::getFloat(const char* const szKey, float fDefault) const
{
    auto ret = getString(szKey);
    return ret.size() > 0 ? std::atof(ret.c_str()) : fDefault;
}

bool 
Properties::getBoolean(const char* const szKey, bool bDefaut) const
{
    auto ret = getString(szKey);
    return ret.size() > 0 ? !_stricmp(ret.c_str(), "true") : bDefaut;
}

void 
Properties::setInteger(const char* const szKey, int iValue) const
{
    auto value = std::to_string(iValue);
    if (!d->add(szKey, value.c_str()))
        d->update(szKey, value.c_str());
}

void 
Properties::setBoolean(const char* const szKey, bool bValue) const
{
    std::lock_guard<std::mutex> locker(d->mutex);
    auto value = bValue ? "true" : "false";
    if (!d->add(szKey, value))
        d->update(szKey, value);
}

bool 
Properties::load(const char* const szFileName) const
{
    std::lock_guard<std::mutex> locker(d->mutex);
    std::ifstream prop;

    prop.open(szFileName, std::ios_base::in);
    if (prop.is_open()) {
        std::string line;

        d->clear();
        while ( !prop.eof() ) {
            std::size_t  id;
            
            std::getline(prop, line);
            if (line.size() == 0)
                continue;
            if (line[0] == '#')
                continue;
            id = line.find_first_of('=');
            if (id != std::string::npos) {
                auto key = line.substr(0, id);
                auto value = line.substr(id + 1);
                key = std::trim(key);                
                value = std::trim(value);
                d->properties[key] = value;
                d->add(key.c_str(), value.c_str());
            }
        }
        return true;
    }
    return false;
}

bool 
Properties::save(const char* const szFileName) const
{
    std::lock_guard<std::mutex> locker(d->mutex);
    std::ofstream prop;

    prop.open(szFileName, std::ios_base::out);
    if (prop.is_open()) {
        for (auto it = d->properties.begin(); it != d->properties.end(); ++it) {
            prop << (*it).first << "=" << (*it).second << std::endl;
        }
        return true;
    }
    return false;
}