#ifndef ENCODING_HPP_INCLUDED
#define ENCODING_HPP_INCLUDED ENCODING_HPP_INCLUDED

#include <string>
#include <stdexcept>
#include <cstdint>

namespace enc {

class Error: public std::runtime_error {
public:
    Error(std::string const& msg): std::runtime_error(msg) { }
};

typedef std::basic_string<std::uint16_t> ucs2str;


ucs2str utf8ToUcs2(std::string const& utf8);
std::string ucs2ToUtf8(ucs2str const& ucs2);

std::wstring ucs2ToWideChar(ucs2str const& ucs2);
ucs2str wideCharToUcs2(std::wstring const& w);

std::string wideCharToUtf8(std::wstring const& w);
std::wstring utf8ToWideChar(std::string const& utf8);

#ifdef _WIN32
std::string ucs2ToAnsi(ucs2str const& w);
ucs2str ansiToUcs2(std::string const& w);
#endif

} // namespace enc

#endif //ENCODING_HPP_INCLUDED
