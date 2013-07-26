// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "comp/RectCollisionComponent.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "LuaEventHelpers.hpp"

static char const libname[] = "RectCollisionComponent";
#include "ExportThis.hpp"


JD_BASIC_COMPONENT_IMPL(RectCollisionComponent)

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS RectCollisionComponent
        class_<LHCURCLASS, Component, WeakRef<Component>>("RectCollisionComponent")
            .def(constructor<Entity&>())
            .JD_EVENT(collided, Collided)

#       undef LHCURCLASS
    ];
}
