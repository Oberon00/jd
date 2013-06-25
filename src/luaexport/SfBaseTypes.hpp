// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef SF_BASE_TYPES_HPP_INCLUDED
#define SF_BASE_TYPES_HPP_INCLUDED SF_BASE_TYPES_HPP_INCLUDED

#include <luabind/object.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/String.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <boost/locale/encoding_utf.hpp>


typedef sf::Vector2<lua_Number> LuaVec2;
typedef sf::Vector3<lua_Number> LuaVec3;
typedef sf::Rect<lua_Number> LuaRect;

namespace luaSfGeo {
    extern char const* const libname;

    template <typename T>
    struct Traits;

    template <>
    struct Traits<LuaVec2>
    {
        static char const* const mtName;
        static char const* const expName;
        static void const* const mmtKey;
    };

    template <>
    struct Traits<LuaVec3>
    {
        static char const* const mtName;
        static char const* const expName;
        static void const* const mmtKey;
    };

    template <>
    struct Traits<LuaRect>
    {
        static char const* const mtName;
        static char const* const expName;
        static void const* const mmtKey;
    };

    template <typename T>
    T* to(lua_State* L, int idx)
    {
        return static_cast<T*>(luaL_checkudata(L, idx, Traits<T>::mtName));
    }

    template <typename T>
    T* optTo(lua_State* L, int idx)
    {
        return static_cast<T*>(luaL_testudata(L, idx, Traits<T>::mtName));
    }

    template <typename T>
    T* push(lua_State* L, T const& v)
    {
        T* newvec = static_cast<T*>(
            lua_newuserdata(L, sizeof(T)));
        new (newvec) T(v);
        luaL_setmetatable(L, Traits<T>::mtName);
        return newvec;
    }

} // namespace luaSfGeo

namespace luabind {
#define CONVERTER(luat, cppt) \
    template<typename T>                                        \
    struct default_converter<cppt<T>>:                          \
        public native_converter_base<cppt<T>> {                 \
        static int compute_score(lua_State* L, int index)       \
        {                                                       \
            if (luaL_testudata(L, index, luaSfGeo::Traits<luat>::mtName)) \
                return 1;                                       \
            return -1;                                          \
        }                                                       \
        cppt<T> from(lua_State* L, int index)                   \
        {                                                       \
            assert(luaL_testudata(L, index, luaSfGeo::Traits<luat>::mtName)); \
            return static_cast<cppt<T>>(                        \
                *static_cast<luat*>(lua_touserdata(L, index))); \
        }                                                       \
        void to(lua_State* L, cppt<T> const& x)                 \
        {                                                       \
            luaSfGeo::push<luat>(L, static_cast<luat>(x));      \
        }                                                       \
    };                                                          \
    template <typename T>                                       \
    struct default_converter<cppt<T> const&>                    \
      : default_converter<cppt<T>> { };

CONVERTER(LuaVec2, sf::Vector2)
CONVERTER(LuaVec3, sf::Vector3)
CONVERTER(LuaRect, sf::Rect)

#undef CONVERTER

// sf::String <-> Lua converter (see luabind/detail/policy.hpp:741)
template <>
    struct default_converter<sf::String>
      : native_converter_base<sf::String>
    {
        static int compute_score(lua_State* L, int index)
        {
            return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
        }

        sf::String from(lua_State* L, int index)
        {
            return sf::String(boost::locale::conv::utf_to_utf<sf::Uint32>(
                lua_tostring(L, index)));
        }

        void to(lua_State* L, sf::String const& value)
        {
            std::string const s(
                boost::locale::conv::utf_to_utf<char>(
                value.getData(), value.getData() + value.getSize()));
            lua_pushlstring(L, s.data(), s.size());
        }
    };

    template <>
    struct default_converter<sf::String const>
      : default_converter<sf::String>
    {};

    template <>
    struct default_converter<sf::String const&>
      : default_converter<sf::String>
    {};
} // namespace luabind

#endif
