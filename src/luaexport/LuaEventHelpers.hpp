// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef LUA_EVENT_HELPERS_HPP_INCLUDED
#define LUA_EVENT_HELPERS_HPP_INCLUDED LUA_EVENT_HELPERS_HPP_INCLUDED

#include "LuaFunction.hpp"
#include <luabind/adopt_policy.hpp>
#include <ssig.hpp>

namespace luabind {
    template <typename Signature>
    struct default_converter<ssig::Connection<Signature>>
      : native_converter_base<ssig::Connection<Signature>>
    {
        void to(lua_State* L, ssig::Connection<Signature> const& value)
        {
            luabind::object o(L, makeConnection(value),
                luabind::adopt(luabind::result));
            o.push(L);
        }
    private:
        ssig::ConnectionBase* makeConnection(
            ssig::Connection<Signature> const& con)
        {
            return new ssig::ScopedConnection<Signature>(con);
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

#define JD_EVENT(name, cname) def("on" #cname, &LHCURCLASS::connect_##name)

#endif // include guard
