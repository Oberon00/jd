// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef LUA_EVENT_HELPERS_HPP_INCLUDED
#define LUA_EVENT_HELPERS_HPP_INCLUDED LUA_EVENT_HELPERS_HPP_INCLUDED

#include "LuaFunction.hpp"

#include <boost/bind.hpp>
#include <ssig.hpp>


#define LUA_EVENT_HELPERS_MAX_ARGS SSIG_MAX_ARGS

#define JD_EVENT_ENTRY(ename, r, ...) \
    if (name == BOOST_STRINGIZE(ename))           \
        return makeConnection(cc->connect_##ename(boost::bind( \
            LuaFunction<r>(recv), __VA_ARGS__))); \
    else { }

#define JD_EVENT_ENTRY0(ename, r) \
    if (name == BOOST_STRINGIZE(ename))          \
        return makeConnection(cc->connect_##ename(boost::bind( \
            &luabind::call_function<r>, recv))); \
    else { }

#define JD_EVENT_TABLE_BEGIN(ct) \
    ssig::ConnectionBase* ct##Meta::connectEvent( \
            lua_State* L,                         \
            Component* c,                         \
            std::string const& name) const        \
    {                                             \
        ct* cc = c->as<ct>();                     \
        luabind::object recv(luabind::from_stack(L, -1)); \
        using boost::ref; using boost::cref;

#define JD_EVENT_TABLE_END return nullptr; }


template<typename Signature>
ssig::ScopedConnection<Signature>* makeConnection(
    ssig::Connection<Signature> const& con)
{
    return new ssig::ScopedConnection<Signature>(con);
}

namespace luabind {
    template <typename Signature>
    struct default_converter<ssig::Connection<Signature>>
      : native_converter_base<ssig::Connection<Signature>>
    {
        void to(lua_State* L, ssig::Connection<Signature> const& value)
        {
            luabind::object o(
                L, static_cast<ssig::ConnectionBase*>(makeConnection(value)));
            o.push(L);
        }
    };

    template <typename Signature>
    struct default_converter<ssig::Connection<Signature> const>
      : default_converter<ssig::Connection<Signature>>
    {};

    template <typename Signature>
    struct default_converter<ssig::Connection<Signature> const&>
      : default_converter<ssig::Connection<Signature>>
    {};
} // namespace luabind

#endif // include guard
