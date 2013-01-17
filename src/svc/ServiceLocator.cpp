#include "svc/ServiceLocator.hpp"

#include "compsys/Component.hpp"
#include "compsys/MetaComponent.hpp"
#include "svc/Configuration.hpp"
#include "svc/DrawService.hpp"
#include "svc/EventDispatcher.hpp"
#include "svc/LuaVm.hpp"
#include "svc/Mainloop.hpp"
#include "svc/SoundManager.hpp"
#include "svc/StateManager.hpp"
#include "svc/Timer.hpp"


/* static */ std::unordered_map<MetaComponent const*, Service*> ServiceLocator::registry;

#define DEFINE_SVC_GETTER(cn, vn) \
    static cn* vn##_ = nullptr;                                   \
    /* static */ cn& ServiceLocator::vn()                         \
    {                                                             \
        if (!vn##_)                                               \
            throw Error("ServiceLocator: no " #cn " registered"); \
        return *vn##_;                                            \
    }

DEFINE_SVC_GETTER(StateManager, stateManager)
DEFINE_SVC_GETTER(LuaVm, luaVm)
DEFINE_SVC_GETTER(Mainloop, mainloop)
DEFINE_SVC_GETTER(EventDispatcher, eventDispatcher)
DEFINE_SVC_GETTER(DrawService, drawService)
DEFINE_SVC_GETTER(Timer, timer)
DEFINE_SVC_GETTER(SoundManager, soundManager)
DEFINE_SVC_GETTER(Configuration, configuration)

/* static */ void ServiceLocator::registerService(Service& s)
{
    registry.insert(std::make_pair(&s.metaComponent(), &s));

#   define ENTRY(cn, vn) \
        if (cn* ss = component_cast<cn>(&s)) \
            vn##_ = ss;

    ENTRY(StateManager, stateManager)
    else ENTRY(LuaVm, luaVm)
    else ENTRY(Mainloop, mainloop)
    else ENTRY(EventDispatcher, eventDispatcher)
    else ENTRY(DrawService, drawService)
    else ENTRY(Timer, timer)
    else ENTRY(SoundManager, soundManager)
    else ENTRY(Configuration, configuration)

#   undef ENTRY
}

/* static */ Component& ServiceLocator::get(MetaComponent const& m)
{
    auto const it = registry.find(&m);
    if (it == registry.end())
        throw Error("ServiceLocator: no Service of type \"" + m.name() + "\" registered");
    return *it->second;
}

