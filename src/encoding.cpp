#include "encoding.hpp"
#include <physfs.h>
#include <vector>
#include <cstdint>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <boost/lexical_cast.hpp>
#endif

#define SMALL_WCHAR WCHAR_MAX - WCHAR_MIN < UINT16_MAX
static_assert(
    sizeof(wchar_t) <= sizeof(std::uint16_t),
    "wchar_t greater than 2 byte not supported.");

namespace enc {

ucs2str utf8ToUcs2(std::string const& utf8) {
    std::vector<PHYSFS_uint16> buf(utf8.size() + 1);
    PHYSFS_utf8ToUcs2(
        utf8.c_str(), &buf[0], buf.size() * sizeof(PHYSFS_uint16));
    return &buf[0];
}

std::string ucs2ToUtf8(ucs2str const& ucs2) {
    std::vector<char> buf(ucs2.size() * 2 + 1);
    PHYSFS_utf8FromUcs2(
        ucs2.c_str(),
        &buf[0],
        buf.size());
    return &buf[0];
}

std::string ucs2ToUtf8(std::wstring const& ucs2) {
    std::vector<char> buf(ucs2.size() * 2 + 1);
    PHYSFS_utf8FromUcs2(
        reinterpret_cast<PHYSFS_uint16 const*>(ucs2.c_str()),
        &buf[0],
        buf.size());
    return &buf[0];
}

std::wstring utf8ToWideChar(std::string const& utf8)
{
#if SMALL_WCHAR
    return std::wstring(utf8.begin(), utf8.end();
#else
    return ucs2ToWideChar(utf8ToUcs2(utf8));
#endif
}

std::string wideCharToUtf8(std::wstring const& w)
{
#if SMALL_WCHAR
    return std::string(w.begin(), w.end();
#else
    return ucs2ToUtf8(wideCharToUcs2(w));
#endif

}

std::wstring ucs2ToWideChar(ucs2str const& ucs2)
{
#if SMALL_WCHAR
    return utf8ToWideChar(ucs2ToUtf8(ucs2));
#else
    return std::wstring(ucs2.begin(), ucs2.end());
#endif
}

ucs2str wideCharToUcs2(std::wstring const& w)
{
#if SMALL_WCHAR
    return utf8ToUcs2(wideCharToUtf8(w));
#else
    return ucs2str(w.begin(), w.end());
#endif
}

#ifdef _WIN32
static_assert(
    sizeof(PHYSFS_uint16) == sizeof(wchar_t),
    "wchar_t has the wrong size");

std::string ucs2ToAnsi(ucs2str const& ucs2)
{
    std::vector<char> multiByte(ucs2.size() * 2 + 1);
    BOOL usedDefaultChar = true;
    if (WideCharToMultiByte(
            CP_ACP, WC_COMPOSITECHECK,
            reinterpret_cast<wchar_t const*>(ucs2.c_str()), -1,
            &multiByte[0], multiByte.size(),
            nullptr, &usedDefaultChar) == 0 || usedDefaultChar) {
        if (usedDefaultChar) {
            throw Error(
                "Cannot encode given UCS-2 string '"
                + ucs2ToUtf8(ucs2)
                + "' in current codepage.");
        } else {
            throw Error("WideCharToMultiByte failed: " +
                        boost::lexical_cast<std::string>(GetLastError()));
        }
    }
    return &multiByte[0];
}

ucs2str ansiToUcs2(std::string const& ansi)
{
    std::vector<wchar_t> wideChar(ansi.size() + 1);
    if (MultiByteToWideChar(
        CP_ACP, MB_PRECOMPOSED,
            ansi.c_str(), -1,
            &wideChar[0], wideChar.size()) == 0) {
        throw Error("MultiByteToWideChar failed: " +
                    boost::lexical_cast<std::string>(GetLastError()));
    }
    return reinterpret_cast<std::uint16_t*>(&wideChar[0]);
}

std::wstring utf8ToFstreamArg(std::string const& utf8)
{
    return utf8ToWideChar(utf8);
}

#endif

} // namespace enc