#include "svc/Timer.hpp"

#include "compsys/BasicMetaComponent.hpp"

static char const libname[] = "Timer";
#include "ExportThis.hpp"


JD_BASIC_COMPONENT_IMPL(Timer)

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
        class_<LHCURCLASS, Component>("Timer")
            .def("callEvery", &Timer_callEvery)
            .def("callAfter", &Timer_callAfter)
            .LHPROPG(frameDuration)
            .property("factor", &LHCURCLASS::factor, &LHCURCLASS::setFactor)
            .scope [
                class_<LHCURCLASS::CallOrder, ConnectionBase>("CallOrder")
            ]

#       undef LHCURCLASS
    ];
}
