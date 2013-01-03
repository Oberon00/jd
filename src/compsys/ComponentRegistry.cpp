#include "ComponentRegistry.hpp"

#include "compsys/MetaComponent.hpp"

extern "C" {
#   include <lauxlib.h>
#   include <lua.h>
}

#include <cassert>

static char const* libname = "ComponentRegistry";
#include "luaexport/ExportThis.hpp"


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
    assert(lua_type(L, -1) == LUA_TUSERDATA);
    auto& result = *static_cast<ComponentRegistry*>(lua_touserdata(L, -1));
	lua_pop(L, 1);
	return result;
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

/* static */ void ComponentRegistry::_init(lua_State* L)
{
	void* registryLocation = lua_newuserdata(L, sizeof (ComponentRegistry)); // allocate memory
	assert(registryLocation);

    // create metatable
    lua_createtable(L, 0, 1);

    // metatable["__gc"] = &destroyComponentRegistry
    lua_pushliteral(L, "__gc");
    lua_pushcfunction(L, &destroyComponentRegistry);
    lua_rawset(L, -3);

    lua_setmetatable(L, -2);


    // create ComponentRegistry
    new(registryLocation) ComponentRegistry;

    lua_rawsetp(L, LUA_REGISTRYINDEX, &get());
}

static void init(LuaVm& vm)
{
	ComponentRegistry::_init(vm.L());
}
