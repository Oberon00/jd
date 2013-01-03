#ifndef LUA_EVENT_HELPERS_HPP_INCLUDED
#define LUA_EVENT_HELPERS_HPP_INCLUDED LUA_EVENT_HELPERS_HPP_INCLUDED

#include "LuaFunction.hpp"
#include "ssig.hpp"

#include <boost/bind.hpp>


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
    ConnectionBase* ct##Meta::connectEvent( \
            lua_State* L,                   \
            Component* c,                   \
            std::string const& name) const  \
    {                                       \
        ct* cc = c->as<ct>();               \
        luabind::object recv(luabind::from_stack(L, -1)); \
        using boost::ref; using boost::cref;

#define JD_EVENT_TABLE_END return nullptr; }


template<typename Signature>
ScopedConnection<Signature>* makeConnection(Connection<Signature> const& con)
{
    return new ScopedConnection<Signature>(con);
}

#endif // include guard
