// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED CONTAINER_HPP_INCLUDED

#include <luabind/class.hpp>
#include <luabind/dependency_policy.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/object.hpp>

#include <stdexcept>
#include <type_traits>


struct lua_State;

namespace containerexport_detail {

template<bool ref, typename T>
typename std::enable_if<ref, T*>::type makeRef(T& t) { return &t; }

template<bool ref, typename T>
typename std::enable_if<!ref, T>::type makeRef(T& t) { return t; }

template<typename C>
void AssocContainer_set(C& c, typename C::key_type const& key, luabind::argument value)
{
    if (luabind::type(value) == LUA_TNIL)
        c.erase(key);
    else
        c[key] = luabind::object_cast<typename C::mapped_type>(value);
}

template<typename C, bool ref>
luabind::object AssocContainer_get(C& c, lua_State* L, typename C::key_type const& key)
{
   typename C::iterator it = c.find(key);
   if (it == c.end())
       return luabind::object();
   return luabind::object(L, makeRef<ref>(it->second));
}

template<typename C, bool ref>
auto RandomAccessContainer_get(C& c, typename C::size_type i)
    -> decltype(makeRef<ref>(c.at(i - 1)))
{
    if (i == 0)
        throw std::out_of_range("index starts at 1");
    return makeRef<ref>(c.at(i - 1));
}


template<typename C>
void RandomAccessContainer_set(C& c, typename C::size_type i, luabind::argument value)
{
    if (i == 0)
        throw std::out_of_range("index starts at 1");

    --i;
    if (luabind::type(value) == LUA_TNIL) {
        typedef typename C::difference_type offset_t;
        if (c.size() > i)
            c.erase(c.begin() + static_cast<offset_t>(i));
        else
            throw std::out_of_range("attempt to set invalid index to nil");
    } else {
        c.at(i) = luabind::object_cast<typename C::value_type>(value);
    }
}

template <typename T>
T& identity(T& t)
{
    return t;
}

} // namespace containerexport_detail

template <typename C>
void exportContainer(luabind::class_<C>& c)
{
    using namespace luabind;
    c
        .def(constructor<>())
        .property("isEmpty", &C::empty)
        .def("clear", &C::clear)
        .def("iter", &containerexport_detail::identity<C>,
            dependency(_1, result) + return_stl_iterator);
}

template <bool ref, typename C>
void exportRandomAccessContainer(luabind::class_<C>& c)
{
     exportContainer(c);
     c
         .def(luabind::constructor<typename C::size_type>())
         .def("get", &containerexport_detail::RandomAccessContainer_get<C, ref>)
         .def("set", &containerexport_detail::RandomAccessContainer_set<C>)
         .def("add", (void(C::*)(typename C::const_reference))&C::push_back)
         .property("count", &C::size, (void(C::*)(typename C::size_type))&C::resize);
}

template <bool ref, typename C>
void exportAssocContainer(luabind::class_<C>& c)
{
    exportContainer(c);
    c
        .scope [
            luabind::class_<typename C::value_type>("Pair")
                .def_readonly("key", &C::value_type::first)
                .def_readwrite("value", &C::value_type::second)
        ]
        .def("set", &containerexport_detail::AssocContainer_set<C>)
        .def("get", &containerexport_detail::AssocContainer_get<C, ref>)
        .property("count", &C::size);
}

#endif
