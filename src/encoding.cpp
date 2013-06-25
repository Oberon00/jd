// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "encoding.hpp"

#include <boost/locale/encoding_utf.hpp>

namespace enc {

#ifdef BOOST_MSVC

std::wstring utf8ToFstreamArg(std::string const& utf8)
{
    return boost::locale::conv::utf_to_utf<wchar_t>(utf8);
}

#endif

} // namespace enc
