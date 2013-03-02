#ifndef SHAREDPTR_CONVERTER_HPP_INCLUDED
#define SHAREDPTR_CONVERTER_HPP_INCLUDED SHAREDPTR_CONVERTER_HPP_INCLUDED

#include <boost/version.hpp>


#if BOOST_VERSION >= 105300

#include <boost/get_pointer.hpp>


namespace luabind { namespace detail { namespace has_get_pointer_ {
    using boost::get_pointer;
}}}

#else // if BOOST_VERSION < 105300

#include <memory>


// Not standard conforming: add function to ::std(::tr1)
namespace std {
#if defined(_MSC_VER) && _MSC_VER < 1700
namespace tr1 {
#endif
template<class T>
T * get_pointer(shared_ptr<T> const& p) { return p.get(); }

template<class T>
T const* get_pointer(shared_ptr<T const> const& p) { return p.get(); }
#if defined(_MSC_VER) && _MSC_VER < 1700
} // namespace tr1
#endif
} // namespace std

#endif // if BOOST_VERSION < 105300

#endif
