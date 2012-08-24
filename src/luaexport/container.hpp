#ifndef CONTAINER_HPP_INCLUDED
#define CONTAINER_HPP_INCLUDED CONTAINER_HPP_INCLUDED

#include <luabind/class.hpp>
#include <luabind/object.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/dependency_policy.hpp>
#include <stdexcept>


struct lua_State;

namespace containerexport_detail {

template<typename C>
void AssocContainer_set(C& c, typename C::key_type const& key, luabind::argument value)
{
    if (luabind::type(value) == LUA_TNIL)
        c.erase(key);
    else
        c[key] = luabind::object_cast<C::mapped_type>(value);
}

template<typename C>
luabind::object AssocContainer_get(C& c, lua_State* L, typename C::key_type const& key)
{
   typename C::iterator it = c.find(key);
   if (it == c.end())
       return luabind::object();
   return luabind::object(L, it->second);
}

template<typename C>
luabind::object RandomAccessContainer_get(C& c, lua_State* L, typename C::size_type i)
{
    if (i == 0)
        throw std::out_of_range("index starts at 1");
    return luabind::object(L, c.at(i - 1));
}


template<typename C>
void RandomAccessContainer_set(C& c, typename C::size_type i, luabind::argument value)
{
    if (i == 0)
        throw std::out_of_range("index starts at 1");

    --i;
    if (luabind::type(value) == LUA_TNIL) {
        if (c.size() > i)
            c.erase(c.begin() + i);
        else
            throw std::out_of_range("attempt to set invalid index to nil");
    } else {
        c.at(i) = luabind::object_cast<C::value_type>(value);
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

template <typename C>
void exportRandomAccessContainer(luabind::class_<C>& c)
{
     exportContainer(c);
     c
         .def(luabind::constructor<typename C::size_type>())
         .def("get", &containerexport_detail::RandomAccessContainer_get<C>)
         .def("set", &containerexport_detail::RandomAccessContainer_set<C>)
         .def("add", (void(C::*)(typename C::const_reference))&C::push_back)
         .property("count", &C::size, (void(C::*)(typename C::size_type))&C::resize);
}

template <typename C>
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
        .def("get", &containerexport_detail::AssocContainer_get<C>)
        .property("count", &C::size);
}

#endif
