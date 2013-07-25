// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/EventDispatcher.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "LuaEventHelpers.hpp"
#include "SfBaseTypes.hpp"

static char const libname[] = "EventDispatcher";
#include "ExportThis.hpp"


JD_BASIC_EVT_COMPONENT_IMPL(EventDispatcher)

JD_EVENT_TABLE_BEGIN(EventDispatcher)
#define E0(n) JD_EVENT_ENTRY0(n, void)
#define E1(n) JD_EVENT_ENTRY(n, void, _1)
    E0(closed)
    E1(resized)
    E0(lostFocus)
    E0(gainedFocus)
    E1(textEntered)

    E1(keyPressed)
    E1(keyReleased)

    E1(mouseWheelMoved)
    E1(mouseButtonPressed)
    E1(mouseButtonReleased)
    E1(mouseMoved)
    E0(mouseEntered)
    E0(mouseLeft)
#undef E0
#undef E1
JD_EVENT_TABLE_END

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS EventDispatcher
        class_<LHCURCLASS, Component>(BOOST_STRINGIZE(LHCURCLASS))
            .LHPROPG(isWindowFocused)
            .LHMEMFN(isKeyPressed)
            .LHMEMFN(isMouseButtonPressed)
            .LHMEMFN(mousePosition)
    ];
}
