#pragma once

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <string>
#include <sstream>
#include <vector>
#include <Windows.h>

#if UNICODE
typedef std::wstring       xstring;
typedef std::wstringstream xstringstream;
typedef wchar_t            xchar;
#else
typedef std::string        xstring;
typedef std::stringstream  xstringstream;
typedef char               xchar;
#endif
typedef std::vector<xstring>  StringList;

namespace std
{
    static int is_space(int ch)
    {
        return ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r';
    }

    static inline std::string &ltrim(std::string &s) 
    {
        s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::is_space))));
        return s;
    }

    // trim from end
    static inline std::string &rtrim(std::string &s) 
    {
        s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::is_space))).base(), s.end());
        return s;
    }

    // trim from both ends
    static inline std::string &trim(std::string &s) 
    {
        return ltrim(rtrim(s));
    }

    inline xstring toString(LPCSTR text)
    {
        xstring result;

        if (text == nullptr)
            return result;
#if UNICODE
        auto len = strlen(text);
        if (len == 0)
            return result;
        auto uniname = new wchar_t[len * sizeof(wchar_t) + 10];
        auto ret = MultiByteToWideChar(CP_UTF8, 0, text, len, uniname, len * sizeof(wchar_t));
        uniname[ret] = static_cast<wchar_t>(0);
        result = xstring(uniname);
        delete[] uniname;
#else
        result = xstring(text);
#endif
        return result;
    }

    inline xstring toString(const std::string &text)
    {
        return toString(text.c_str());
    }

    inline std::string toString(LPCWSTR lpwstrString)
    {
#define BUFFER_TAIL  256
        std::string result;
        auto len = lstrlenW(lpwstrString);
        auto buf = new char [len * sizeof(WCHAR) + 10];
        auto ret = WideCharToMultiByte(CP_UTF8, 0, lpwstrString, len, buf, len * sizeof(WCHAR), nullptr, nullptr);
        if (ret == 0) {
            auto err = GetLastError();
            fprintf(stdout, "Error: %d\n", err);
        }
        buf[ret] = '\0';
        result = std::string(buf);
        delete[] buf;
        return result;
    }

    inline std::string toString(const std::wstring &text)
    {
        return toString(text.c_str());
    }

    inline std::vector<xstring> &split(const xstring &s, xchar delim, StringList &elems) {
        xstringstream ss(s);
        xstring item;
        while (std::getline(ss, item, delim)) {
            elems.push_back(item);
        }
        return elems;
    }


    inline StringList split(const xstring &s, xchar delim) {
        StringList elems;
        split(s, delim, elems);
        return elems;
    }
}
