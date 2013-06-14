#include "comp/RectCollisionComponent.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "LuaEventHelpers.hpp"

static char const libname[] = "RectCollisionComponent";
#include "ExportThis.hpp"


JD_BASIC_EVT_COMPONENT_IMPL(RectCollisionComponent)

JD_EVENT_TABLE_BEGIN(RectCollisionComponent)
    JD_EVENT_ENTRY(collided, void, _1, ref(_2), _3)
JD_EVENT_TABLE_END

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS RectCollisionComponent
        class_<LHCURCLASS, Component, WeakRef<Component>>("RectCollisionComponent")
            .def(constructor<Entity&>())

#       undef LHCURCLASS
    ];
}
