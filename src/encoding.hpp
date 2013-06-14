#ifndef ENCODING_HPP_INCLUDED
#define ENCODING_HPP_INCLUDED ENCODING_HPP_INCLUDED

#include <string>
#include <boost/config.hpp>

namespace enc {

#ifdef BOOST_MSVC
std::wstring utf8ToFstreamArg(std::string const& utf8);
#else
inline std::string const& utf8ToFstreamArg(std::string const& utf8) { return utf8; }
#endif

} // namespace enc

#endif //ENCODING_HPP_INCLUDED
