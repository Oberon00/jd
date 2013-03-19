#include "luaUtils.hpp"
#include "svc/FileSystem.hpp"

#include <physfs.h>
#include <zlib.h>
#include <cstdint>

static char const libname[] = "LuaIo";
#include "ExportThis.hpp"

static int serialize(lua_State* L)
{
    try {
        std::string result("return ");
        while(int const top = lua_gettop(L)) {
            result += luaU::serialize(L, 1);
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
        f.close();
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
        std::vector<char> buf(static_cast<std::size_t>(sz));
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

static void throwZErr(int r)
{
    if (r != Z_OK)
        throw std::runtime_error(zError(r));
}

typedef std::uint32_t uncompressed_len_t;

static int compressString(lua_State* L)
{
    size_t srcLen;
    char const* s = luaL_checklstring(L, 1, &srcLen);
    try {
        std::vector<char> buf(
            std::max(
                static_cast<size_t>(srcLen * 1.001 + 12 + 1),
                static_cast<size_t>(1)) +
            sizeof(uncompressed_len_t));
        *reinterpret_cast<uncompressed_len_t*>(&buf[0]) = srcLen;
        uLongf dstLen = buf.size() - sizeof(uncompressed_len_t);
        throwZErr(compress(
            reinterpret_cast<Bytef*>(&buf[sizeof(uncompressed_len_t)]), &dstLen,
            reinterpret_cast<Bytef const*>(s), srcLen));
        lua_pushlstring(L, &buf[0], dstLen + sizeof(uncompressed_len_t));
        return 1;
    } catch (std::exception const& e) {
        return luaL_error(L, "could not compress data: %s", e.what());
    }
}

static int uncompressString(lua_State* L)
{
    size_t srcLen;
    char const* s = luaL_checklstring(L, 1, &srcLen);
    luaL_argcheck(L, srcLen >= sizeof(uncompressed_len_t), 1, "invalid data");
    try {
        std::vector<char> buf(std::max(
            *reinterpret_cast<uncompressed_len_t const*>(s),
            static_cast<uncompressed_len_t>(1)));
        uLongf dstLen = buf.size();
        throwZErr(uncompress(
            reinterpret_cast<Bytef*>(&buf[0]), &dstLen,
            reinterpret_cast<Bytef const*>(
                s + sizeof(uncompressed_len_t)), srcLen - sizeof(uncompressed_len_t)));
        lua_pushlstring(L, &buf[0], dstLen);
        return 1;
    } catch (std::exception const& e) {
        return luaL_error(L, "could not uncompress data: %s", e.what());
    }
}

static int fileExists(lua_State* L)
{
    lua_pushboolean(L, PHYSFS_exists(luaL_checkstring(L, 1)));
    return 1;
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
        {"fileExists",  &fileExists},
        {"compress",    &compressString},
        {"uncompress",  &uncompressString},
        {nullptr, nullptr}
    };

    lua_getglobal(L, "jd");
    luaL_setfuncs(L, iofuncs, 0);
}
