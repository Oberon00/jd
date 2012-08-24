#ifndef LUA_VECTOR_HPP_INCLUDED
#define LUA_VECTOR_HPP_INCLUDED LUA_VECTOR_HPP_INCLUDED

#include <luabind/object.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Rect.hpp>

typedef sf::Vector2<lua_Number>   LuaVec2;
typedef sf::Vector3<lua_Number>   LuaVec3;
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
} // namespace luabind

#endif
