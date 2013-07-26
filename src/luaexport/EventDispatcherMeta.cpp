// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/EventDispatcher.hpp"

#include "LuaEventHelpers.hpp"
#include "SfBaseTypes.hpp"

static char const libname[] = "EventDispatcher";
#include "ExportThis.hpp"

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS EventDispatcher
        LHCLASS
            .LHPROPG(isWindowFocused)
            .LHMEMFN(isKeyPressed)
            .LHMEMFN(isMouseButtonPressed)
            .LHMEMFN(mousePosition)
            .JD_EVENT(closed, Closed)
            .JD_EVENT(resized, Resized)
            .JD_EVENT(lostFocus, LostFocus)
            .JD_EVENT(gainedFocus, GainedFocus)
            .JD_EVENT(textEntered, TextEntered)
            .JD_EVENT(keyPressed, KeyPressed)
            .JD_EVENT(keyReleased, KeyReleased)
            .JD_EVENT(mouseWheelMoved, MouseWheelMoved)
            .JD_EVENT(mouseButtonPressed, MouseButtonPressed)
            .JD_EVENT(mouseButtonReleased, MouseButtonReleased)
            .JD_EVENT(mouseMoved, MouseMoved)
            .JD_EVENT(mouseEntered, MouseEntered)
            .JD_EVENT(mouseLeft, MouseLeft)
    ];
}
