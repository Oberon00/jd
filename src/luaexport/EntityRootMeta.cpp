#include "svc/EntityRoot.hpp"
#include "compsys/Entity.hpp"
#include "compsys/BasicMetaComponent.hpp"

static char const libname[] = "EntityRoot";
#include "ExportThis.hpp"

JD_SINGLETON_COMPONENT_IMPL(EntityRoot)

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    vm.initLib("EntitySystem");
    LHMODULE [
#       define LHCURCLASS EntityRoot
        class_<LHCURCLASS, bases<Component, EntityCollection>>(BOOST_STRINGIZE(LHCURCLASS))
    ];
}
