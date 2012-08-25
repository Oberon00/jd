#include "svc/ServiceLocator.hpp"
#include "compsys/ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"
#include "svc/StateManager.hpp"
#include "compsys/Component.hpp"
#include "svc/Mainloop.hpp"
#include "svc/EventDispatcher.hpp"
#include "svc/DrawService.hpp"
#include "svc/Timer.hpp"
#include "Collisions.hpp"

static char const libname[] = "ServiceLocator";
#include "ExportThis.hpp"

static luabind::object getService(lua_State* L, std::string const& name)
{
    MetaComponent const& mc = ComponentRegistry::metaComponent(name);
    mc.castUp(L, &ServiceLocator::get(mc));
    luabind::object result(luabind::from_stack(L, -1));
    lua_pop(L, 1);
    return result;
}

static void init(LuaVm& vm)
{
    LHMODULE [ namespace_("svc") [
#define LHCURCLASS ServiceLocator
        LHMEMFN(stateManager),
        LHMEMFN(mainloop),
        LHMEMFN(drawService),
        LHMEMFN(eventDispatcher),
        LHMEMFN(collisionManager),
        LHMEMFN(timer),
        def("get", &getService)
    ] ];
}
