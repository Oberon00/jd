#include "Logfile.hpp"
char const libname[] = "Logfile";
#include "ExportThis.hpp"

static void writelog(lua_State* L, std::string const& msg, loglevel::T lv)
{
    // modified from luaL_where
    lua_Debug ar;
    if (lua_getstack(L, 1, &ar)) {  /* check function at level */
        lua_getinfo(L, "Sl", &ar);  /* get info about it */
        if (ar.currentline > 0) {  /* is there info? */
            log().write(msg, lv,
                lua_pushfstring(L, "%s:%d", ar.short_src, ar.currentline));
            lua_pop(L, 1);
            return;
        }
    }
    // end

    log().write(msg, lv, "<unknown/Lua>");
}

static void logd(lua_State* L, std::string const& msg)
{
    writelog(L, msg, loglevel::debug);
}

static void logi(lua_State* L, std::string const& msg)
{
    writelog(L, msg, loglevel::info);
}

static void logw(lua_State* L, std::string const& msg)
{
    writelog(L, msg, loglevel::warning);
}

static void loge(lua_State* L, std::string const& msg)
{
    writelog(L, msg, loglevel::error);
}

static void logwf(lua_State* L, std::string const& msg)
{
    writelog(L, msg, loglevel::fatal);
}

static void init(LuaVm& vm)
{
    LHMODULE [
        namespace_("log") [
            def("d", &logd),
            def("i", &logi),
            def("w", &logw),
            def("e", &loge),
            def("f", &logwf)
        ]
    ];
}
