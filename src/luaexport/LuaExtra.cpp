#include "luaUtils.hpp"

static char const libname[] = "LuaExtra";
#include "ExportThis.hpp"


static int debugId(lua_State* L)
{
    void const* p = lua_topointer(L, 1);
    if (!p) {
        p = lua_touserdata(L, 1); // light userdata
        if (!p) {
            return luaL_argerror(L, 1,
                "must be a table, (light) userdata, function or thread");
        }
    }
    lua_pushfstring(L, "%p", p);
    return 1;
}

static void init(LuaVm& vm)
{
    lua_State* const L = vm.L();
    LUAU_BALANCED_STACK(L);
    lua_getglobal(L, "debug");
    if (lua_isnil(L, -1))
        return;
    lua_pushcfunction(L, &debugId);
    lua_setfield(L, -2, "id");
}
