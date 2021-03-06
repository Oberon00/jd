// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "LuaVm.hpp"

#include "FileSystem.hpp"
#include "jdConfig.hpp"
#include "Logfile.hpp"
#include "luaUtils.hpp"

#include <luabind/lua_include.hpp>
#include <luabind/open.hpp>
extern "C" {
#   include <lualib.h>
}

#include <array>


static int pcallf(lua_State* L)
{
    luaL_traceback(L, L, lua_tostring(L, -1), 1);
    return 1;
}

static lua_CFunction oldpanicf = nullptr;

static int panicf(lua_State* L)
{
    pcallf(L);
    LOG_F(luaU::Error(L, "Lua panic").what());
    return oldpanicf(L);
}


static char const registryKey = '\0';

/* static */ LuaVm& LuaVm::get(lua_State* L)
{
    lua_rawgetp(L, LUA_REGISTRYINDEX, &registryKey);
    LuaVm* vm = static_cast<LuaVm*>(lua_touserdata(L, -1));
    if (!vm)
        throw luaU::Error("no LuaVm registered for the requested state");
    return *vm;
}

LuaVm::LuaVm(std::string const& libConfigFilename)
{
    m_L = luaL_newstate();
    oldpanicf = lua_atpanic(m_L, panicf);

    lua_newtable(m_L);
    lua_pushvalue(m_L, -1);
    lua_setglobal(m_L, jd::moduleName);

    lua_pushliteral(m_L, "DEBUG");

#ifdef NDEBUG
    lua_pushboolean(m_L, false);
#else
    lua_pushboolean(m_L, true);
#endif
    lua_rawset(m_L, -3);
    lua_pop(m_L, 1);

    lua_pushlightuserdata(m_L, this);
    lua_rawsetp(m_L, LUA_REGISTRYINDEX, &registryKey);

    auto const require = [this](lua_CFunction f, char const* n)->void {
        luaL_requiref(m_L, n, f, n[0] ? true : false);
        lua_pop(m_L, 1);
    };
    require(luaopen_base, "");
    luabind::open(m_L);
    luabind::set_pcall_callback(&pcallf);

    int const rbegin = lua_gettop(m_L) + 1;
    luaU::load(m_L, libConfigFilename);
    luaU::pcall(m_L, 0, LUA_MULTRET);
    int const rend = lua_gettop(m_L) + 1;

    for (int i = rbegin; i < rend; ++i) {
        if (lua_type(m_L, i) != LUA_TSTRING) {
            LOG_W("Value " + luaU::dumpvar(m_L, i) + " returned by " +
                libConfigFilename + " is not a string.");
            continue;
        }

        char const* libname = lua_tostring(m_L, i);
        assert(libname);
#       define ENTRY(ln) if (strcmp(#ln, libname) == 0) { require(luaopen_##ln, #ln); }
        ENTRY(coroutine)
        else ENTRY(package)
        else ENTRY(string)
        else ENTRY(table)
        else ENTRY(math)
        else ENTRY(bit32)
        else ENTRY(io)
        else ENTRY(os)
        else ENTRY(debug)
#       undef ENTRY
        else LOG_W("Unknown Lua library name \"" + std::string(libname) +
            "\" returned by " + libConfigFilename);
    }
    lua_settop(m_L, rbegin - 1);
}

LuaVm::~LuaVm()
{
   lua_pushglobaltable(m_L);
   lua_pushliteral(m_L, "jd");
   lua_pushnil(m_L);
   lua_rawset(m_L, -3);
   lua_pop(m_L, 1);

    try { deinit(); }
    catch (std::exception const& e) {
        LOG_E("LuaVm::deinit() failed with following exception:");
        LOG_EX(e);
    }

    if (lua_gettop(m_L) != 0)
        LOG_W("Elements left on Lua stack: " + luaU::dumpstack(m_L));

    lua_State* L = m_L;
    m_L = nullptr;
    lua_close(L);
}

/* static */ void LuaVm::registerLib(std::string const& libname, LibInitFn const& initFn)
{
   bool const success = libRegistry().insert(std::make_pair(libname, LibInfo(initFn))).second;
   (void)success;
   assert(success);
}

namespace {
struct ResetInit {
    ResetInit(bool& initialized): m_initialized(initialized), m_commit(false) {
        assert(!m_initialized);
        m_initialized = true;
    }
    ~ResetInit() {
        if (!m_commit)
            m_initialized = false;
    }
    void commit() {
        m_commit = true;
    }
private:
    ResetInit& operator= (ResetInit const&);

    bool& m_initialized;
    bool m_commit;
};
} // anonymous namespace

void LuaVm::initLib(std::string const& libname)
{
    auto const it = libRegistry().find(libname);
    if (it == libRegistry().end())
        throw luaU::Error("library \"" + libname + "\" not found");
    if (!it->second.initialized) {
        ResetInit r(it->second.initialized);
        it->second.initFn(*this);
        r.commit();
    }
}

void LuaVm::initLibs()
{
    for (auto& lib : libRegistry())
        if (!lib.second.initialized) {
            ResetInit r(lib.second.initialized);
            lib.second.initFn(*this);
            r.commit();
        }
}

static void collectgarbage(lua_State* L)
{
    lua_pushcfunction(L, [](lua_State* L) -> int {
        lua_gc(L, LUA_GCCOLLECT, 0);
        return 0;
    });
    luaU::pcall(L, 0, 0);
}

void LuaVm::deinit()
{
    // Try to avoid crashes caused by the undefined order
    // in which lua_close calls finalizers.

    collectgarbage(m_L);

    lua_pushglobaltable(m_L);
    lua_pushliteral(m_L, "jd");
    lua_pushnil(m_L);
    lua_rawset(m_L, -3);
    collectgarbage(m_L);


    // clear package.loaded and .preload //
    lua_pushliteral(m_L, "package");
    lua_rawget(m_L, -2);

    if (lua_isnil(m_L, -1)) {
        // already cleared.
        lua_pop(m_L, 2); // nil, globals table
        return;
    }

    lua_pushliteral(m_L, "loaded");
    lua_rawget(m_L, -2);

    luaU::cleartable(m_L, -1);

    lua_pushliteral(m_L, "preload");
    lua_rawget(m_L, -2);
    luaU::cleartable(m_L, -1);

    lua_pop(m_L, 1); // pop package table


    // clear globals table //
    luaU::cleartable(m_L, -1);

    collectgarbage(m_L);
}
