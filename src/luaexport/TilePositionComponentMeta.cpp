// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "comp/TilePositionComponent.hpp"

#include "compsys/BasicMetaComponent.hpp"
#include "compsys/Entity.hpp"
#include "LuaEventHelpers.hpp"
#include "SfBaseTypes.hpp"
#include "Tilemap.hpp"

static char const libname[] = "TilePositionComponent";
#include "ExportThis.hpp"


JD_BASIC_EVT_COMPONENT_IMPL(TilePositionComponent)

JD_EVENT_TABLE_BEGIN(TilePositionComponent)
    JD_EVENT_ENTRY(tilePositionChanged, void, _1, _2)
JD_EVENT_TABLE_END

static void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [
#       define LHCURCLASS TilePositionComponent
        class_<LHCURCLASS, Component, WeakRef<Component>>("TilePositionComponent")
            .def(constructor<Entity&, jd::Tilemap const&, unsigned>())
            .def(constructor<Entity&, jd::Tilemap const&>())
            .LHPROPG(tilePosition)
#       undef LHCURCLASS
    ];
}
