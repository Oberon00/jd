#include "svc/EventDispatcher.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "Geometry.hpp"
#include "LuaEventHelpers.hpp"
#include "svc/ServiceLocator.hpp"

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

static EventDispatcher* dsp = nullptr;

static bool EventDispatcher_isKeyPressed(unsigned k)
{
    if (!dsp)
        dsp = &ServiceLocator::eventDispatcher();
    return dsp->isKeyPressed(static_cast<sf::Keyboard::Key>(k));
}

static bool EventDispatcher_isMouseButtonPressed(unsigned b)
{
    if (!dsp)
        dsp = &ServiceLocator::eventDispatcher();
    return dsp->isMouseButtonPressed(static_cast<sf::Mouse::Button>(b));
}

static LuaVec2 EventDispatcher_mousePosition()
{
    if (!dsp)
        dsp = &ServiceLocator::eventDispatcher();
    return static_cast<LuaVec2>(dsp->mousePosition());
}

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
        namespace_("kb") [
            def("isKeyPressed", &EventDispatcher_isKeyPressed)
        ],
        namespace_("mouse") [
            def("position", &EventDispatcher_mousePosition),
            def("isButtonPressed", &EventDispatcher_isMouseButtonPressed)
        ],
#       define LHCURCLASS EventDispatcher
        class_<LHCURCLASS, Component>(BOOST_STRINGIZE(LHCURCLASS))
            .LHPROPG(isWindowFocused)  
    ];
}
