#include "ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"
#include <cassert>
#include <boost/foreach.hpp>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
}

void ComponentRegistry::registerComponent(MetaComponent const* meta)
{
    assert(meta);
    bool const success = m_registry.insert(std::make_pair(meta->name(), meta)).second;
    assert(success);
}

MetaComponent const* ComponentRegistry::operator[] (std::string const& name)
{
    auto it = m_registry.find(name);
    if (it == m_registry.end())
        return nullptr;
    assert(it->second->name() == name);
    return it->second;
}

/* static */ ComponentRegistry& ComponentRegistry::get()
{
    static ComponentRegistry cppRegistry;
    return cppRegistry;
}

static int destroyComponentRegistry(lua_State* L)
{
#ifndef NDEBUG
    luaL_checktype(L, 1, LUA_TUSERDATA);
#endif
    ComponentRegistry* r = static_cast<ComponentRegistry*>(lua_touserdata(L, 1));
    r->~ComponentRegistry();
    return 0;
}

/* static */ ComponentRegistry& ComponentRegistry::get(lua_State* L)
{
    lua_pushlightuserdata(L, &get()); // adress of the C++ ComponentRegistry is the key
    lua_rawget(L, LUA_REGISTRYINDEX); // to the Lua registry in L
    
    // Registry for this state exists yet?
    if (lua_type(L, -1) == LUA_TUSERDATA)
        return *static_cast<ComponentRegistry*>(lua_touserdata(L, -1));

    assert(lua_type(L, -1) == LUA_TNIL);

    void* registryLocation = lua_newuserdata(L, sizeof (ComponentRegistry)); // allocate memory

    // create metatable
    lua_createtable(L, 0, 1);

    // metatable["__gc"] = &destroyComponentRegistry
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &destroyComponentRegistry);
    lua_rawset(L, -3);

    lua_setmetatable(L, -2);


    // create ComponentRegistry
    ComponentRegistry* newRegistry = new(registryLocation) ComponentRegistry;

    lua_rawsetp(L, LUA_REGISTRYINDEX, &get());
    return *newRegistry;
}

MetaComponent const& ComponentRegistry::metaComponent(std::string const& name, lua_State* L)
{
    MetaComponent const* mc;
    if (L) {
        mc = get(L)[name];
        if (mc)
            return *mc;
    }
    mc = get()[name];
    if (!mc)
        throw InvalidMetaComponentName(name);
    return *mc;
}
