// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifdef LUA_EXPORT_HELPERS_HPP_INCLUDED
#   error "Double inclusion of ExportThis.hpp is an error!"
#endif

#define LUA_EXPORT_HELPERS_HPP_INCLUDED LUA_EXPORT_HELPERS_HPP_INCLUDED

#include "jdConfig.hpp"
#include "LuaExportMacros.hpp"
#include "svc/LuaVm.hpp"

#include <luabind/luabind.hpp>


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
