static char const libname[] = "LuaIo";
#include "ExportThis.hpp"
#include "svc/FileSystem.hpp"
#include "LuaUtils.hpp"
#include <physfs.h>
#include <boost/iostreams/stream.hpp>

namespace io = boost::iostreams;

typedef io::stream<VFileDevice> VFileStream;

static unsigned maxserializationdepth = 100;

static std::string quoteLuaString(lua_State* L, int idx)
{
    assert(L);

    size_t len;
    char const* s = lua_tolstring(L, idx, &len);
    std::string r;
    r.reserve(len + 3);
    
    r += '"';

    for (std::size_t i = 0; i < len; ++i) {
        char const c = s[i];
#define OR || c ==
        if (c == '\a' OR '\b' OR '\f' OR '\n' OR '\r' OR '\t' OR '\v' OR '\\' OR '"') {
#undef OR
            r += '\\';
            r += c;
        } else if (c == '\0' || iscntrl(c)) {
            r += '\\';
            std::string const n = std::to_string(static_cast<int>(c));
            assert(n.size() <= 3);
            if (i < len - 1 && isdigit(s[i + 1]))
                r += std::string(3 - n.size(), '0');
            r += n;
        } else {
            r += c;
        }
    }

    r += '"';

    return r;
}

static bool getSerializeCallback(lua_State* L, int idx)
{
    int const t = lua_type(L, idx);
    if (t != LUA_TTABLE && t != LUA_TUSERDATA)
        return false;
    if (luaL_getmetafield(L, idx, "__serialize_raw")) {
        lua_pop(L, 1);
        return false;
    }
    return luaL_getmetafield(L, idx, "__serialize")
        || luaL_getmetafield(L, idx, "__tostring");
}


static std::string callSerializeCallback(lua_State* L, int obj)
{
    LUAU_BALANCED_STACK2(L, -1);
    lua_pushvalue(L, obj);
    luaU::pcall(L, 1, 1);
    return lua_tostring(L, -1);
}

static std::string serializeUdata(lua_State* L, int idx)
{
    LUAU_BALANCED_STACK(L);
    idx = lua_absindex(L, idx);
    if (!getSerializeCallback(L, idx))
        throw std::runtime_error("userdata has no serialization callback");
    return callSerializeCallback(L, idx);
}

static std::string serializeTable(lua_State* L, int idx, unsigned depth);

static std::string serialize(lua_State* L, int idx, unsigned depth)
{
    std::string r;
    switch(lua_type(L, idx)) {
        case LUA_TSTRING: r = quoteLuaString(L, idx); break;
        case LUA_TNUMBER: r = lua_tostring(L, idx); break;
        case LUA_TNIL: r = "nil"; break;
        case LUA_TBOOLEAN: r = lua_toboolean(L, idx) ? "true" : "false"; break;
        case LUA_TTABLE: r = serializeTable(L, idx, depth); break;
        case LUA_TUSERDATA: r = serializeUdata(L, idx); break;
        default:
            throw std::runtime_error(
                std::string("cannot serialize a ") + luaL_typename(L, idx));
    }
    lua_remove(L, idx);
    return r;

}

static std::string serializeTable(lua_State* L, int idx, unsigned depth)
{
    LUAU_BALANCED_STACK(L);
    if (depth > maxserializationdepth)
        throw std::runtime_error("serialization depth too high");

    idx = lua_absindex(L, idx);
    if (getSerializeCallback(L, idx))
        return callSerializeCallback(L, idx);

    std::string r(1, '{');
	// see http://www.lua.org/manual/5.2/manual.html#lua_next
    lua_pushnil(L);
    unsigned nextSeqKey = 1;
    while (luaU::next(L, idx) != 0) {
        if (nextSeqKey > 0
            && lua_isnumber(L, -2)
            && lua_tonumber(L, -2) == nextSeqKey) {
            ++nextSeqKey;
        } else {
            nextSeqKey = 0;
            lua_pushvalue(L, -2); // preserve key
		    r += '[';
            r += serialize(L, -1, depth + 1);
            r += "]=";
        }
        r += serialize(L, -1, depth + 1);
        r += ',';
    }
    r += '}';
    return r;
}

static int serialize(lua_State* L)
{
    try {
        std::string result("return ");
        while(int const top = lua_gettop(L)) {
            result += serialize(L, 1, 0);
            if (top > 1)
                result += ',';

        }
        lua_pushlstring(L, result.c_str(), result.size());
        return 1;
    } catch (std::exception const& e) {
        return luaL_error(L, "serialization failed: %s", e.what());
    }
}

static int writeString(lua_State* L)
{
    char const* filename = luaL_checkstring(L, 1);
    size_t len;
    char const* data = luaL_checklstring(L, 2, &len);
    try {
        VFile f(filename, VFile::openW);
        f.write(data, len);
        f.throwError();
        return 0;
    } catch (std::exception const& e) {
        return luaL_error(L, "writing data failed: %s", e.what());
    }
}

static int readString(lua_State* L)
{
    char const* filename = luaL_checkstring(L, 1);
    try {
        VFile f(filename);
        sf::Int64 sz = f.getSize();
        f.throwError();
        assert(sz >= 0);
        std::vector<char> buf(sz);
        f.read(&buf[0], buf.size());
        f.throwError();
        f.close();

        // push string using pcall: out of memory is not too unlikely here.
        lua_pushcfunction(L, ([](lua_State* L) -> int {
            std::vector<char> const& buf =
                *static_cast<std::vector<char>*>(lua_touserdata(L, 1));
            lua_pushlstring(L, &buf[0], buf.size());
            return 1;
        }));
        lua_pushlightuserdata(L, &buf);
        luaU::pcall(L, 1, 1);

        return 1;
    } catch (std::exception const& e) {
        return luaL_error(L, "reading data failed: %s", e.what());
    }
}

static int createDirectory(lua_State* L)
{
    if (!PHYSFS_mkdir(luaL_checkstring(L, 1)))
        luaL_error(L, "could not create directory: %s", PHYSFS_getLastError());
    return 0;
}

void init(LuaVm& vm)
{
    lua_State* L = vm.L();
    LUAU_BALANCED_STACK(L);
    static luaL_Reg const iofuncs[] = {
        {"serialize",   &serialize},
        {"writeString", &writeString},
        {"readString",  &readString},
        {"createDirectory", &createDirectory},
        {nullptr, nullptr}
    };

    lua_getglobal(L, "jd");
    luaL_setfuncs(L, iofuncs, 0);
}
