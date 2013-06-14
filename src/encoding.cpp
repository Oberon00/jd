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