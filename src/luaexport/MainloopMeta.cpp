// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/Mainloop.hpp"

#include "LuaEventHelpers.hpp"

static char const libname[] = "Mainloop";
#include "ExportThis.hpp"

static void quit0(Mainloop& ml)
{
    ml.quit(); // use default value for exitcode
}

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS Mainloop
        LHCLASS
            .LHMEMFN(quit)
            .def("quit", &quit0)
            .JD_EVENT(started, Started)
            .JD_EVENT(preFrame, PreFrame)
            .JD_EVENT(processInput, ProcessInput)
            .JD_EVENT(update, Update)
            .JD_EVENT(interact, Interact)
            .JD_EVENT(preDraw, PreDraw)
            .JD_EVENT(draw, Draw)
            .JD_EVENT(postDraw, PostDraw)
            .JD_EVENT(postFrame, PostFrame)
            .JD_EVENT(quitting, Quitting)
            .JD_EVENT(quitRequested, QuitRequested)
    ];
}
