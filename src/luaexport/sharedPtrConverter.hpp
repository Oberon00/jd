// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef SHAREDPTR_CONVERTER_HPP_INCLUDED
#define SHAREDPTR_CONVERTER_HPP_INCLUDED SHAREDPTR_CONVERTER_HPP_INCLUDED

#include <boost/version.hpp>

#if BOOST_VERSION >= 105300
#include <boost/get_pointer.hpp>
#include <luabind/detail/has_get_pointer.hpp>


namespace luabind { namespace detail { namespace has_get_pointer_ {
  template<class T>
  struct impl<std::shared_ptr<T>> {
      BOOST_STATIC_CONSTANT(bool, value = true);
      typedef boost::mpl::bool_<value> type;
  };

  template<class T>
  struct impl<const std::shared_ptr<T>>: impl<std::shared_ptr<T>> { };

  template<class T>
  struct impl<volatile std::shared_ptr<T>>: impl<std::shared_ptr<T>> { };

  template<class T>
  struct impl<const volatile std::shared_ptr<T>>: impl<std::shared_ptr<T>> { };
}}
using boost::get_pointer;
}



#else // if BOOST_VERSION < 105300

// Not standard conforming: add function to ::std(::tr1)
namespace std {

#if defined(_MSC_VER) && _MSC_VER < 1700
namespace tr1 {
#endif

    template<class T>
    T * get_pointer(shared_ptr<T> const& p) { return p.get(); }

#if defined(_MSC_VER) && _MSC_VER < 1700
} // namespace tr1
#endif

} // namespace std

#endif // if BOOST_VERSION < 105300

#endif
