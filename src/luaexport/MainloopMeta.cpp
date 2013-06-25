// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#define MAINLOOP_KEEP_CALLBACKS
#include "svc/Mainloop.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "LuaEventHelpers.hpp"

static char const libname[] = "Mainloop";
#include "ExportThis.hpp"


JD_BASIC_EVT_COMPONENT_IMPL(Mainloop)

JD_EVENT_TABLE_BEGIN(Mainloop)
#define ENTRY(n) JD_EVENT_ENTRY0(n, void)
   CALLBACKS(ENTRY)
#undef ENTRY
   JD_EVENT_ENTRY(quitting, void, _1)
   JD_EVENT_ENTRY(quitRequested, void, _1)
JD_EVENT_TABLE_END

static void quit0(Mainloop& ml)
{
    ml.quit(); // use default value for exitcode
}

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS Mainloop
        class_<LHCURCLASS, Component>(BOOST_STRINGIZE(LHCURCLASS))
            .LHMEMFN(quit)
            .def("quit", &quit0)
    ];
}
