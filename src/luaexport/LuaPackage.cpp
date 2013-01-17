#include "Logfile.hpp"
#include "LuaUtils.hpp"
#include "svc/FileSystem.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <physfs.h>

static char const libname[] = "LuaPackage";
#include "ExportThis.hpp"


static std::string const searchPath(
    std::string name, std::string path,
    char const* sep = ".", char const* rep = "/")
{
    using namespace boost::algorithm;
    if (sep)
        replace_all(name, sep, rep);
    replace_all(path, "?", name);

    std::vector<boost::iterator_range<std::string::iterator>> pelems;
    split(pelems, path, [](char c) { return c == ';'; });

    // hack: The C++ standard does not guarantee that std::string is zero
    //  terminated, only the return value of c_str() is. But even after calling
    //  it, this is not guaranted (or is it?) but any sane implementation of
    //  std::string will do it.
    path.c_str();
    for (auto& elem : pelems) {
        if (elem.end() != path.end()) 
            *elem.end() = '\0';
        char const* filename = &*elem.begin();

        // Note that only the existence, not the actual readability of the file
        // is checked. Trying to open a file using PhysFS could be rather
        // expensive.
        if (PHYSFS_exists(filename))
            return std::string(elem.begin(), elem.end());
    }
    replace_all(path, std::string(1, '\0'), ";"); // print all elements in error message
    throw luaU::Error(
        "none of the files in path could be opened:\n\t\t" + path);
}

static int findPackageInPhysFs(lua_State* L)
{
    char const* name = luaL_checkstring(L, 1);

    lua_getfield(L, lua_upvalueindex(1), "path");

    char const* path = lua_tostring(L, -1);
    if (!path)
        luaL_error(L, "\"package.path\" must be a string");

    try {
        std::string const filename = searchPath(name, path);
        luaU::load(L, filename); // push chunk
        lua_pushlstring(L, filename.c_str(), filename.size()); // push filename
    } catch (std::exception const& e) {
        lua_pushfstring(L, "\n\t%s", e.what());
        return 1;
    }
    return 2;
}

static int searchPathL(lua_State* L)
{
    char const *name = luaL_checkstring(L, 1),
               *path = luaL_checkstring(L, 2),
               *sep  = luaL_optstring  (L, 3, "."),
               *rep  = luaL_optstring  (L, 4, "/");
    try {
        searchPath(name, path, sep, rep);
    } catch (std::exception const& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }
    return 1;
}

static int loadfile(lua_State* L)
{
    char const *filename = lua_tostring  (L, 1),
               *mode     = luaL_optstring(L, 2, "bt");

    luaL_argcheck(L, filename, 1, "loading from stdin not supported");

    try {
         luaU::load(L, filename, mode);
    } catch (std::exception const& e) {
        lua_pushnil(L);
        lua_pushstring(L, e.what());
        return 2;
    }

    // set _ENV
    if (!lua_isnone(L, 3)) {
        lua_pushvalue(L, 3);
        lua_setupvalue(L, -1, 1);
    }
    return 1;
}

static int dofilecont (lua_State *L) {
    int oldtop;
    int const r = lua_getctx(L, &oldtop);
    assert(r == LUA_YIELD);
    (void)r;
    return lua_gettop(L) - oldtop;
}

static int dofile(lua_State* L)
{
    // Note: this function (obviously) accepts the same parameters
    // as loadfile and not, as the Lua manual says, only a filename.
    // In addition, filename is not optional.
    int r = loadfile(L);
    if (r != 1 || !lua_isfunction(L, -1))
        return r;
    int const oldtop = lua_gettop(L) - 1;
    lua_callk(L, 0, LUA_MULTRET, oldtop, dofilecont);
    return lua_gettop(L) - oldtop;
}

void init(LuaVm& vm)
{
    lua_State* L = vm.L();

    LUAU_BALANCED_STACK(L);

    lua_pushcfunction(L, &loadfile);
    lua_setglobal(L, "loadfile");

    lua_pushcfunction(L, &dofile);
    lua_setglobal(L, "dofile");

    lua_getglobal(L, "package");
    if (lua_isnil(L, -1))
        return;

    lua_pushnil(L);
    lua_setfield(L, -2, "loadlib");

    lua_pushnil(L);
    lua_setfield(L, -2, "cpath");

    lua_pushcfunction(L, &searchPathL);
    lua_setfield(L, -2, "searchpath");

    lua_pushliteral(L,
        "lua/?.lua;" "lua/?.luac;"
        "lua/lib/?.lua;" "lua/lib/?.luac");
    lua_setfield(L, -2, "path");

    lua_getfield(L, -1, "searchers");

    // leave preload searcher as it is.

    // replace Lua searcher
    lua_pushvalue(L, -2); // package table --> upvalue #1
    lua_pushcclosure(L, &findPackageInPhysFs, 1);
    lua_rawseti(L, -2, 2);

    // block C and all-in-one searchers
    lua_pushnil(L);
    lua_rawseti(L, -2, 3);
    lua_pushnil(L);
    lua_rawseti(L, -2, 4);
}
