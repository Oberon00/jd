#include "Collisions.hpp"
#include "compsys/Component.hpp"
#include "compsys/ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"
#include "svc/Configuration.hpp"
#include "svc/DrawService.hpp"
#include "svc/EventDispatcher.hpp"
#include "svc/Mainloop.hpp"
#include "svc/ServiceLocator.hpp"
#include "svc/SoundManager.hpp"
#include "svc/StateManager.hpp"
#include "svc/Timer.hpp"

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
        LHMEMFN(timer),
        LHMEMFN(soundManager),
        LHMEMFN(configuration),
        def("get", &getService)
    ] ];
}
