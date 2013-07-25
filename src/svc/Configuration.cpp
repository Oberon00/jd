// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "Configuration.hpp"

#include "Logfile.hpp"
extern "C" {
#   include "lua.h"
}
#include "svc/FileSystem.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/range/adaptor/reversed.hpp>

static char const libname[] = "Configuration";
#include "luaexport/ExportThis.hpp"

Configuration::Configuration(lua_State* L):
    m_conf(luabind::newtable(L))
{ }

void Configuration::load(
    std::string const& configurationFilename,
    std::string const& globalName)
{
    LOG_D("Configuration filename is \"" + configurationFilename + "\".");
    m_confPath = configurationFilename;
    reload();
    luabind::globals(m_conf.interpreter())[jd::moduleName][globalName] = m_conf;
}

void Configuration::reload()
{
    lua_State* L = m_conf.interpreter();
    if (m_conf) {
        LUAU_BALANCED_STACK(L);
        m_conf.push(L);
        luaU::cleartable(L, -1);
    }

    luaU::load(L, m_confPath);

    luabind::object chunk(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    try { m_conf = chunk(); }
    catch (luabind::error const& e) {
        throw Error(luaU::Error(e, "loading configuration failed").what());
    }
    if (luabind::type(m_conf) != LUA_TTABLE) {
        std::string const objrep = luaU::dumpvar(L, -1);
        throw Error(m_confPath + " did not return a table but " + objrep);
    }
}

void Configuration::save()
{
    lua_State* L = m_conf.interpreter();
    m_conf.push(L);
    std::string s = "return ";
    s += luaU::serialize(L, -1);
    VFile f(m_confPath, VFile::openW);
    f.write(s.data(), static_cast<sf::Int64>(s.size()));
    f.close(); // Close manually to avoid swallowing errors.
}

// PARAMS: table, keys in reverse order
// RESULT: table[keys]
static int getTable(lua_State* L)
{
    while (lua_gettop(L) > 1) {
        lua_gettable(L, 1);
        lua_replace(L, 1);
    }
    return 1;
}

luabind::object Configuration::getObject(std::string const& p)
{
    if (!m_conf.is_valid())
        throw Error(p, "Configuration has not been loaded yet.");

    std::vector<boost::iterator_range<std::string::const_iterator>> pelems;
    bool (*splitter)(char) = [](char c) {return c == '.'; };
    boost::split(pelems, p, splitter);
    lua_State* L = m_conf.interpreter();
    if (!lua_checkstack(L, static_cast<int>(pelems.size() + 2)))
        throw Error(p, "Cannot reserve lua stack size. Path to long?");
    lua_pushcfunction(L, &getTable);
    m_conf.push(L);
    for (auto const& elem: boost::adaptors::reverse(pelems))
        lua_pushlstring(L, &*elem.begin(),
            static_cast<std::size_t>(elem.size()));
    try { luaU::pcall(L, static_cast<int>(pelems.size() + 1), 1); }
    catch (luaU::Error const& e) {
        throw Error(p, e.what());
    }
    luabind::object result(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    return result;
}

static void init(LuaVm& vm)
{
    LHMODULE [
#       define LHCURCLASS Configuration
        LHCLASS
            .LHMEMFN(reload)
            .LHMEMFN(save)
    ];
}
