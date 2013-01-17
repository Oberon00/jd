#include "LuaUtils.hpp"

#include "svc/FileSystem.hpp"

#include <boost/lexical_cast.hpp>
#include <luabind/lua_include.hpp>

#include <array>


namespace {
static int doDumpVar(lua_State* L)
{
    std::string* const str = static_cast<std::string*>(lua_touserdata(L, 1));
    assert(str);

    std::size_t len;
    luaL_tolstring(L, 2, &len);
    str->assign(lua_tostring(L, -1), len);
    lua_pop(L, 1);
    return 0;
}

} // anonymous namespace


namespace luaU {

Error::Error(lua_State* L, std::string const& msg):
    std::runtime_error(msg + ": " + lua_tostring(L, -1))
{
    lua_pop(L, 1);
}

Error::Error(luabind::error const& e, std::string const& msg):
    std::runtime_error(
        msg +
        (msg.empty() ? "" : ": ") +
        e.what() + ": " + lua_tostring(e.state(), -1))
{
    lua_pop(e.state(), 1);
}

std::string const dumpvar(lua_State* L, int idx)
{
    int const absidx = lua_absindex(L, idx);
    lua_pushcfunction(L, luabind::get_pcall_callback());
    lua_pushcfunction(L, &doDumpVar);
    std::string varrep;
    lua_pushlightuserdata(L, &varrep);
    lua_pushvalue(L, absidx);
    int r = lua_pcall(L, 2, 0, -4);
    if (r != LUA_OK) {
        varrep = errstring(r) + ": " + lua_tostring(L, -1);
        lua_pop(L, 1);
    }
    lua_pop(L, 1); // pop pcall callback
    std::string const typerep = luaL_typename(L, absidx);
    if (r == LUA_OK)
        return typerep + '(' + varrep + ')';
    else
        return typerep + "[! " + varrep + " !]";
}

std::string const errstring(int luaerr)
{
#define ENTRY(n) case LUA_ERR##n: return "LUA_ERR" #n;
    switch(luaerr) {
        case LUA_OK: return "LUA_OK";
        case LUA_YIELD: return "LUA_YIELD";
        ENTRY(MEM)
        ENTRY(RUN)
        ENTRY(SYNTAX)
        ENTRY(ERR)
        ENTRY(FILE)
        ENTRY(GCMM)
    }
#undef ENTRY
    return "unknown lua error " + boost::lexical_cast<std::string>(luaerr);
}

void pcall(lua_State* L, int nargs, int nresults)
{
    int const msghidx = lua_absindex(L, -nargs - 1);
    lua_pushcfunction(L, luabind::get_pcall_callback());
    lua_insert(L, msghidx); // move beneath arguments and function
    int const r = lua_pcall(L, nargs, nresults, msghidx);
    lua_remove(L, msghidx);
    if (r != LUA_OK)
        throw luaU::Error(L, "lua_pcall failed (" + errstring(r) + ')');

}

bool next(lua_State* L, int idx)
{
    idx = lua_absindex(L, idx);
    lua_pushcfunction(L, [](lua_State* L) -> int {
        return lua_next(L, 1) ? 2 : 0;
    });
    lua_pushvalue(L, idx);
    lua_pushvalue(L, -3); // push previous top (key)
    lua_remove(L, -4);
    luaU::pcall(L, 2, 2);
    if (lua_isnil(L, -2)) { // key is nil --> end
        lua_pop(L, 2);
        return false;
    }
    return true;
}


std::string const dumpstack(lua_State* L)
{
    int const top = lua_gettop(L);
    std::string result;
    result += boost::lexical_cast<std::string>(top) + " elements on stack:\n";
    for (int i = 1; i <= top; i++)
        result += '\t' + luaU::dumpvar(L, i) + '\n';
    result += '\n';
    return result;
}

void exportenum(lua_State* L, int idx, ExportedEnumValue const* entry)
{
    assert(entry);
    assert(lua_istable(L, idx));

    idx = lua_absindex(L, idx);

    while (entry->name) {
        lua_pushstring(L, entry->name);   // key
        lua_pushinteger(L, entry->value); // value
        lua_rawset(L, idx);
        ++entry;
    }

    assert(entry->value == 0);
}

namespace {

struct LoadInfo {
    LoadInfo(std::string const& vfilename): f(vfilename, VFile::openR) { }
    VFile f;
    std::array<char, LUAL_BUFFERSIZE> buf;
};

static const char* loader(lua_State*, void* ud, std::size_t* sz)
{
    assert(ud);
    assert(sz);
    LoadInfo& loadinfo = *static_cast<LoadInfo*>(ud);
    sf::Int64 r = loadinfo.f.read(&loadinfo.buf[0], loadinfo.buf.size());
    if (r >= 0) {
        *sz = static_cast<std::size_t>(r);
        return &loadinfo.buf[0];
    }
    *sz = 0;
    return nullptr;
}

static int dumper(lua_State*, const void* p, std::size_t sz, void* ud)
{
    assert(p);
    assert(ud);
    VFile& f = *static_cast<VFile*>(ud);
    return f.write(p, sz) == sz ? 0 : -1;
}


} // anonymous namespace

void load(lua_State* L, std::string const& vfilename, char const* mode)
{
    LoadInfo loadInfo(vfilename);
    int const r = lua_load(L, &loader, &loadInfo, ('@' + vfilename).c_str(), mode);
    if (r != LUA_OK) {
        if (!loadInfo.f.lastError().empty()) {
            throw luaU::Error(__FUNCTION__ " failed: I/O error: " + loadInfo.f.lastError());
        } else {
            throw luaU::Error(L, 
                __FUNCTION__ " failed: lua_load failed (" + luaU::errstring(r)
                + ")");
        }
    }

    // chunk is now on top of the stack
    assert(lua_isfunction(L, -1));
}

void dumpFunction(lua_State* L, VFile& f)
{
    assert(lua_isfunction(L, -1));
    assert(!lua_iscfunction(L, -1));

    int const r = lua_dump(L, &dumper, &f);
    if (r != LUA_OK) {
        if (!f.lastError().empty()) {
            throw luaU::Error(__FUNCTION__ " failed: I/O error: " + f.lastError());
        } else {
            throw luaU::Error(L, 
                __FUNCTION__ " failed: lua_dump failed (" + luaU::errstring(r)
                + ")");
        }
    }
}

void dumpFunction(lua_State* L, std::string const& vfilename)
{
    VFile f(vfilename, VFile::openW);
    return dumpFunction(L, f);
}

void exec(
    lua_State* L,
    std::string const& vfilename, char const* mode,
    int nargs, int nresults)
{
    load(L, vfilename, mode);
    pcall(L, nargs, nresults);
}

StackBalance::StackBalance(lua_State* L, int diff, Action action):
    m_L((assert(L), L)),
    m_desiredTop(lua_gettop(L) + diff),
    m_action(action)
{    
    assert(m_desiredTop >= 0);

    // When adjusting, debug makes no sense because
    // all errors are corrected automatically.
    assert(!(action & adjust) || !(action & debug));
}

StackBalance::~StackBalance()
{
    int const top = lua_gettop(m_L);
    if (top > m_desiredTop) {
        if (m_action & pop)
            lua_settop(m_L, m_desiredTop);
        else if (m_action & debug)
            assert(!"unbalanced stack: too much elements");
    } else if (top < m_desiredTop) {
        if (m_action & pushNil)
            lua_settop(m_L, m_desiredTop);
       else if (m_action & debug)
            assert(!"unbalanced stack: too less elements");
    }
}


} // namespace luaU
