#ifndef SHAREDPTR_CONVERTER_HPP_INCLUDED
#define SHAREDPTR_CONVERTER_HPP_INCLUDED SHAREDPTR_CONVERTER_HPP_INCLUDED

// Non standard conforming: add function to ::std::tr1
namespace std { namespace tr1 {
template<class T>
T * get_pointer(shared_ptr<T> const& p) { return p.get(); }

template<class T>
T const* get_pointer(shared_ptr<T const> const& p) { return p.get(); }
} } // namespace std::tr1

#endif
