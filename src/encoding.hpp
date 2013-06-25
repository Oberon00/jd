// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

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
