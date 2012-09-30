#include "comp/TileCollisionComponent.hpp"
#include "compsys/BasicMetaComponent.hpp"
#include "LuaEventHelpers.hpp"

static char const libname[] = "TileCollisionComponent";
#include "ExportThis.hpp"

JD_SINGLETON_EVT_COMPONENT_IMPL(TileCollisionComponent)

JD_EVENT_TABLE_BEGIN(TileCollisionComponent)
    JD_EVENT_ENTRY(collided,  void, _1, ref(_2), _3)
    JD_EVENT_ENTRY(overridden, void, _1, ref(_2))
JD_EVENT_TABLE_END

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS TileCollisionComponent
        class_<LHCURCLASS, Component, WeakRef<Component>>("TileCollisionComponent")
            .def(constructor<Entity&, TileCollideableInfo&>())
			.def(constructor<Entity&>())
            
#       undef LHCURCLASS
    ];
}