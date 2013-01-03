#ifndef SHAREDPTR_CONVERTER_HPP_INCLUDED
#define SHAREDPTR_CONVERTER_HPP_INCLUDED SHAREDPTR_CONVERTER_HPP_INCLUDED

#include <memory>


// Non standard conforming: add function to ::std::tr1
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

#endif
