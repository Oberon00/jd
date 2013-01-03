#include "Configuration.hpp"

#include "compsys/MetaComponent.hpp"
#include "Logfile.hpp"
extern "C" {
#   include "lua.h"
}
#include "LuaUtils.hpp"
#include "svc/ServiceLocator.hpp"

#include <boost/algorithm/string/split.hpp>
#include <boost/range/adaptor/reversed.hpp>


namespace {

class Meta: public MetaComponent {
    virtual std::string const& name() const
    {
        return Configuration::componentName;
    }

    virtual Component* create() const
    {
        return new Configuration();
    }
};

} // anonymous namespace

JD_COMPONENT_IMPL(Configuration, Meta)

void Configuration::load(
    std::string const& configurationFilename,
    std::string const& globalName)
{
    LOG_D("Configuration filename is \"" + configurationFilename + "\".");

    luaU::load(ServiceLocator::luaVm().L(), configurationFilename);
    lua_State* L = ServiceLocator::luaVm().L();

    luabind::object chunk(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    try { m_conf = chunk(); }
    catch (luabind::error const& e) {
        throw luaU::Error(e, "loading configuration failed");
    }
    if (luabind::type(m_conf) != LUA_TTABLE) {
        std::string const objrep = luaU::dumpvar(L, -1);
        throw Error(configurationFilename + " did not return a table but " + objrep);
    }

    luabind::globals(L)[jd::moduleName][globalName] = m_conf;
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
    boost::split(pelems, p, [](char c) {return c == '.'; });
    lua_State* L = m_conf.interpreter();
    if (!lua_checkstack(L, pelems.size() + 2))
        throw Error(p, "Cannot reserve lua stack size. Path to long?");
    lua_pushcfunction(L, &getTable);
    m_conf.push(L);
    for (auto const& elem: boost::adaptors::reverse(pelems))
        lua_pushlstring(L, &*elem.begin(), elem.size());
    try { luaU::pcall(L, pelems.size() + 1, 1); }
    catch (luaU::Error const& e) {
        throw Error(p, e.what());
    }
    luabind::object result(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    return result;
}