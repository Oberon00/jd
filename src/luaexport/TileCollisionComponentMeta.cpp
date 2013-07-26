// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "comp/TileCollisionComponent.hpp"

#include "collision/TileCollideableGroup.hpp"
#include "compsys/BasicMetaComponent.hpp"
#include "compsys/Entity.hpp"
#include "LuaEventHelpers.hpp"

static char const libname[] = "TileCollisionComponent";
#include "ExportThis.hpp"


JD_BASIC_COMPONENT_IMPL(TileCollisionComponent)

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS TileCollisionComponent
        class_<LHCURCLASS, Component, WeakRef<Component>>("TileCollisionComponent")
            .def(constructor<Entity&, TileCollideableInfo&>())
            .def(constructor<Entity&>())
            .JD_EVENT(collided, Collided)
            .JD_EVENT(overridden, Overriden)

#       undef LHCURCLASS
    ];
}
