#include <Windows.h>
#include <sqlite3.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include "xstring.h"
#include "logger.h"
#include "properties.h"

namespace Internal {
    const char * const __p_TableScheme =
        "CREATE TABLE IF NOT EXISTS Properties ("
        "    id           INTEGER PRIMARY KEY AUTOINCREMENT, "
        "    key          TEXT UNIQUE NOT NULL,              "
        "    value        TEXT DEFAULT NULL                  "
        ");";
    const char * const __p_TablesInsertStmt =
        "INSERT INTO Properties(key, value) "
        "  VALUES(?, ?)";
    const char * const __p_TablesUpdateStmt =
        "UPDATE Properties SET value = ? WHERE key = ?";

    const char * const __p_TablesListStmt =
        "SELECT id, key, value FROM Properties ORDER BY key";
    const char * const __p_TablesGetStmt =
        "SELECT id, key, value FROM Properties WHERE key = ?";
    const char * const __p_TablesDeleteStmt =
        "DELETE FROM Properties";
    const char * const __p_TablesDeleteKeyStmt =
        "DELETE FROM Properties WHERE key = ?";
    const char * const __p_TablesCountStmt =
        "SELECT COUNT(id) FROM Properties";

    typedef std::pair<std::string, std::string>  PropertyKey;
    typedef std::vector<PropertyKey>             PropertyList;

    class PropertiesPrivate {
    public:
        explicit PropertiesPrivate(Logger *_logger)
            : db(nullptr), error(nullptr), ret(0), logger(_logger), 
              hHeap(HeapCreate(HEAP_NO_SERIALIZE, 0, 0))
        {
            sqlite3_initialize();
        }
        ~PropertiesPrivate()
        {
            HeapDestroy(hHeap);
        }
        bool   open();
        bool   close();
        bool   clear();
        int    nextId() const;
        int    count();

        bool   add(const char * const szKey, const char *    const szValue);
        bool   add(const char * const szKey, const wchar_t * const szValue);
        bool   update(const char * const szKey, const char* const szValue);
        bool   update(const char * const szKey, const wchar_t* const szValue);
        bool   remove(const char * const szKey);
        bool   list(PropertyList &list);
        std::string  getString(const char * const szKey);
        std::wstring getWString(const char * const szKey);
    private:
        bool   create();
        void   clear_error();
        wchar_t *alloc(const char * const szBuffer);
        char    *alloc(const wchar_t * const szBuffer);
        char    *toString(const wchar_t * const szBuffer);
        wchar_t *toString(const char * const szBuffer);
        void   free(LPVOID pMemory);

        sqlite3      *db;
        char         *error;
        int           ret;
        Logger       *logger;
        HANDLE        hHeap;
        std::mutex    mutex;
        std::unordered_map<std::string, std::string> properties;
        friend class Properties;
    };

    void PropertiesPrivate::clear_error()
    {
        if (error != nullptr)
            sqlite3_free(error);
        error = nullptr;
    }

    bool PropertiesPrivate::open()
    {
        close();
        ret = sqlite3_open(":memory:", &db);
        if (ret != SQLITE_OK)
            return false;
        return create();
    }

    bool PropertiesPrivate::close()
    {
        if (db)
            sqlite3_close(db);
        db = nullptr;
        clear_error();
        return true;
    }

    bool PropertiesPrivate::clear()
    {
        clear_error();
        ret = sqlite3_exec(db, __p_TablesDeleteStmt, nullptr, nullptr, &error);
        return ret == SQLITE_OK;
    }

    bool PropertiesPrivate::create()
    {
        clear_error();
        ret = sqlite3_exec(db, __p_TableScheme, nullptr, nullptr, &error);
        return ret == SQLITE_OK;
    }

    int PropertiesPrivate::nextId() const
    {
        return static_cast<int>(sqlite3_last_insert_rowid(db));
    }

    int PropertiesPrivate::count()
    {
        sqlite3_stmt *stmt = nullptr;
        int result;
        clear_error();

        ret = sqlite3_prepare_v2(
            db,
            __p_TablesCountStmt,
            std::strlen(__p_TablesCountStmt),
            &stmt,
            nullptr);
        if (ret != SQLITE_OK)
            return -1;
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_ROW) {
            sqlite3_finalize(stmt);
            return -1;
        }
        result = sqlite3_column_int(stmt, 0);
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_DONE)
            result = -1;
        sqlite3_finalize(stmt);
        return result;
    }

    bool PropertiesPrivate::add(const char * const szKey, const char * const szValue)
    {
        sqlite3_stmt *stmt = nullptr;
        clear_error();
        ret = sqlite3_prepare_v2(
            db,
            __p_TablesInsertStmt,
            std::strlen(__p_TablesInsertStmt),
            &stmt,
            nullptr);
        if (ret != SQLITE_OK)
            return false;
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, szKey, std::strlen(szKey), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, szValue, std::strlen(szValue), SQLITE_STATIC);
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_DONE) {
            sqlite3_last_insert_rowid(db);
        }
        sqlite3_finalize(stmt);
        return ret == SQLITE_DONE;
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
        sqlite3_stmt *stmt = nullptr;
        clear_error();
        ret = sqlite3_prepare_v2(
            db,
            __p_TablesUpdateStmt,
            std::strlen(__p_TablesUpdateStmt),
            &stmt,
            nullptr);
        if (ret != SQLITE_OK)
            return false;
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, szValue, std::strlen(szValue), SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, szKey, std::strlen(szKey), SQLITE_STATIC);
        ret = sqlite3_step(stmt);
        if (ret == SQLITE_DONE) {
            sqlite3_last_insert_rowid(db);
        }
        sqlite3_finalize(stmt);
        return ret == SQLITE_DONE;
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
        sqlite3_stmt *stmt = nullptr;
        clear_error();
        ret = sqlite3_prepare_v2(
            db,
            __p_TablesInsertStmt,
            std::strlen(__p_TablesDeleteKeyStmt),
            &stmt,
            nullptr);
        if (ret != SQLITE_OK)
            return false;
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, szKey, std::strlen(szKey), SQLITE_STATIC);
        ret = sqlite3_step(stmt);
        sqlite3_finalize(stmt);
        return ret == SQLITE_DONE;
    }

    bool PropertiesPrivate::list(PropertyList &list)
    {
        sqlite3_stmt *stmt = nullptr;
        clear_error();

        ret = sqlite3_prepare_v2(
            db,
            __p_TablesListStmt,
            std::strlen(__p_TablesListStmt),
            &stmt,
            nullptr);
        if (ret != SQLITE_OK)
            return false;
        while (true) {
            ret = sqlite3_step(stmt);
            if (ret != SQLITE_ROW)
                break;
            auto key = reinterpret_cast<char *>(const_cast<unsigned char *>(sqlite3_column_text(stmt, 1)));
            auto value = reinterpret_cast<char *>(const_cast<unsigned char *>(sqlite3_column_text(stmt, 2)));
            auto p = PropertyKey(key, value);
            list.push_back(p);
        }
        sqlite3_finalize(stmt);
        return ret == SQLITE_DONE;
    }

    char    *PropertiesPrivate::toString(const wchar_t * const szValue)
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

    wchar_t *PropertiesPrivate::toString(const char * const szValue)
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
        std::string result;
        sqlite3_stmt *stmt = nullptr;

        clear_error();
        ret = sqlite3_prepare_v2(
            db,
            __p_TablesGetStmt,
            std::strlen(__p_TablesGetStmt),
            &stmt,
            nullptr);
        if (ret != SQLITE_OK)
            return result;
        sqlite3_reset(stmt);
        sqlite3_bind_text(stmt, 1, szKey, std::strlen(szKey), SQLITE_STATIC);
        ret = sqlite3_step(stmt);
        if (ret != SQLITE_ROW)
            return result;
        result = reinterpret_cast<char *>(const_cast<unsigned char *>(sqlite3_column_text(stmt, 2)));
        sqlite3_finalize(stmt);
        return result;
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

    void PropertiesPrivate::free(LPVOID pMemory)
    {
        if (pMemory != nullptr)
            HeapFree(hHeap, HEAP_NO_SERIALIZE, pMemory);
    }

    wchar_t * PropertiesPrivate::alloc(const char * const szBuffer)
    {
        return static_cast<wchar_t *>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, (std::strlen(szBuffer) + 100) * sizeof(wchar_t)));
    }

    char * PropertiesPrivate::alloc(const wchar_t * const szBuffer)
    {
        return static_cast<char *>(HeapAlloc(hHeap, HEAP_ZERO_MEMORY | HEAP_NO_SERIALIZE, (std::wcslen(szBuffer) + 100) * sizeof(wchar_t)));
    }
}
using namespace Internal;

Properties::Properties(Logger *logger)
: d(new PropertiesPrivate(logger))
{
    d->open();
}

Properties::Properties(struct sqlite3 *db, Logger *logger)
: d(new PropertiesPrivate(logger))
{
    d->db = db;
    d->create();
}

Properties::~Properties()
{
}

std::string 
Properties::getString(const char* const szKey, const char* const szDefault)
{
    std::lock_guard<std::mutex> locker(d->mutex);
    auto ret = d->properties[szKey];// d->getString(szKey);
    return ret.size() > 0 ? ret : std::string(szDefault == nullptr ? "" : szDefault);
}

std::wstring 
Properties::getWString(const char* const szKey, const wchar_t* const szDefault)
{
//    std::lock_guard<std::mutex> locker(d->mutex);
    auto ret = d->getWString(szKey);
    return ret.size() > 0 ? ret : std::wstring(szDefault == nullptr ? L"" : szDefault);
}

void 
Properties::setString(const char* const szKey, const char* const szValue)
{
//    std::lock_guard<std::mutex> locker(d->mutex);
    if (!d->add(szKey, szValue))
        d->update(szKey, szValue);
}

void 
Properties::setString(const char* const szKey, const wchar_t* const szValue)
{
//    std::lock_guard<std::mutex> locker(d->mutex);
    if (!d->add(szKey, szValue))
        d->update(szKey, szValue);
}

int 
Properties::getInteger(const char* const szKey, int iDefault)
{
//    std::lock_guard<std::mutex> locker(d->mutex);
    std::string::size_type sz;
    auto ret = getString(szKey);
    return ret.size() > 0 ? std::stoi(ret, &sz) : iDefault;
}

bool 
Properties::getBoolean(const char* const szKey, bool bDefaut)
{
//    std::lock_guard<std::mutex> locker(d->mutex);
    auto ret = getString(szKey);
    return ret.size() > 0 ? !_stricmp(ret.c_str(), "true") : bDefaut;
}

void 
Properties::setInteger(const char* const szKey, int iValue)
{
//    std::lock_guard<std::mutex> locker(d->mutex);
    auto value = std::to_string(iValue);
    if (!d->add(szKey, value.c_str()))
        d->update(szKey, value.c_str());
}

void 
Properties::setBoolean(const char* const szKey, bool bValue)
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
//        PropertyList list;
//        if (d->list(list)) {
//            for (auto it = list.begin(); it != list.end(); ++it) {
//                auto kv = (*it);
//                prop << kv.first << " = " << kv.second << std::endl;
//            }
//        }
        for (auto it = d->properties.begin(); it != d->properties.end(); ++it) {
            prop << (*it).first << "=" << (*it).second << std::endl;
        }
        return true;
    }
    return false;
}