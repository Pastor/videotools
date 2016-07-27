#pragma once
#include <memory>

struct sqlite3;
class  Logger;

namespace Internal {
    class PropertiesPrivate;
}

class Properties {
    using PropertiesPrivate = Internal::PropertiesPrivate;
public:
    explicit Properties(Logger *logger = nullptr);
    explicit Properties(struct sqlite3 *db, Logger *logger = nullptr);
    virtual ~Properties();

    std::string  getString (const char * const szKey, const char * const szDefault = nullptr) const;
    std::wstring getWString(const char * const szKey, const wchar_t * const szDefault = nullptr) const;

    void         setString (const char * const szKey, const char * const szValue) const;
    void         setString (const char * const szKey, const wchar_t * const szValue) const;

    int          getInteger(const char * const szKey, int iDefault = 0) const;
    float        getFloat(const char * const szKey, float fDefault = .0) const;
    bool         getBoolean(const char * const szKey, bool bDefaut = false) const;
    void         setInteger(const char * const szKey, int iValue) const;
    void         setBoolean(const char * const szKey, bool bValue) const;

    bool load(const char * const szFileName) const;
    bool save(const char * const szFileName) const;
private:
    std::shared_ptr<PropertiesPrivate>  d;
};
