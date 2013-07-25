// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "svc/Timer.hpp"

static char const libname[] = "Timer";
#include "ExportThis.hpp"

static Timer::CallOrder Timer_callEvery(
    Timer& timer, sf::Time every, luabind::object o)
{
    return timer.callEvery(
        every, boost::bind(&luabind::call_function<void>, o));
}

static Timer::CallOrder Timer_callAfter(
    Timer& timer, sf::Time after, luabind::object o)
{
    return timer.callAfter(
        after, boost::bind(&luabind::call_function<void>, o));
}


static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [

#       define LHCURCLASS Timer
        LHCLASS
            .def("callEvery", &Timer_callEvery)
            .def("callAfter", &Timer_callAfter)
            .LHPROPG(frameDuration)
            .property("factor", &LHCURCLASS::factor, &LHCURCLASS::setFactor)
            .scope [
                class_<LHCURCLASS::CallOrder, ssig::ConnectionBase>("CallOrder")
            ]

#       undef LHCURCLASS
    ];
}
