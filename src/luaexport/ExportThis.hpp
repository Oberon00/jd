#ifdef LUA_EXPORT_HELPERS_HPP_INCLUDED
#   error "Double inclusion of ExportThis.hpp is an error!"
#endif

#define LUA_EXPORT_HELPERS_HPP_INCLUDED LUA_EXPORT_HELPERS_HPP_INCLUDED

#include <luabind/luabind.hpp>
#include "svc/LuaVm.hpp"
#include "jdConfig.hpp"
#include "LuaExportMacros.hpp"

#undef  LHMODULE
#define LHMODULE using namespace luabind; module(vm.L(), ::jd::moduleName)

// static char const libname[] = "ModuleName";

static void init(LuaVm& vm);

static bool prepareInit()
{
    LuaVm::registerLib(libname, &init);
    return bool();
}

static bool const dummy = prepareInit();
