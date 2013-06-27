// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "compsys/Component.hpp"
#include "compsys/ComponentRegistry.hpp"
#include "compsys/Entity.hpp"
#include "compsys/MetaComponent.hpp"
#include "container.hpp"
#include "luaUtils.hpp"

#include <luabind/adopt_policy.hpp>
#include <luabind/dependency_policy.hpp>
#include <luabind/iterator_policy.hpp>
#include <luabind/operator.hpp>

char const libname[] = "EntitySystem";
#include "ExportThis.hpp"


static std::vector<luabind::object> getComponents(Entity& this_, lua_State* L)
{
    LUAU_BALANCED_STACK_DBG(L);
    auto const& comps = this_.components();
    std::vector<luabind::object> result;
    for (Component const& c : comps) {
        c.metaComponent().castDown(L, const_cast<Component*>(&c));
        result.push_back(luabind::object(luabind::from_stack(L, -1)));
        lua_pop(L, 1);
    }
    return result;
}

static luabind::object getComponent(Entity& this_, lua_State* L, std::string const& name)
{
    Component* c = this_[ComponentRegistry::metaComponent(name, L)];
    if (!c)
        return luabind::object();
    c->metaComponent().castDown(L, c);
    return luabind::object(luabind::from_stack(L, -1));
}

static luabind::object requireComponent(Entity& this_, lua_State* L, std::string const& name)
{
    Component* c = this_[ComponentRegistry::metaComponent(name, L)];
    if (!c) {
        throw std::runtime_error(
            "required component \"" + name + "\" not available!");
    }
    c->metaComponent().castDown(L, c);
    return luabind::object(luabind::from_stack(L, -1));
}

static std::ostream& operator<< (std::ostream& os, Entity const& e)
{
    os << "jd.Entity ( @" << &e << "; ";
    auto const& comps = e.components();
    if (!comps.empty())
        for (Component const& c : comps)
            os << c.metaComponent().name() << ' ';
    return os << ')';
}

static bool operator==(Entity const& lhs, Entity const* rhs)
    { return &lhs == rhs; }

void init(LuaVm& vm)
{
    LUAU_BALANCED_STACK_DBG(vm.L());

    typedef std::vector<luabind::object> ObjectVec;
    luabind::class_<ObjectVec> cObjectVec("ComponentList");
    exportRandomAccessContainer<false>(cObjectVec);

    vm.initLib("ComponentSystem");
    LHMODULE [

#       define LHCURCLASS Entity
        LHCLASS
            .scope [
                cObjectVec
            ]
            .def(constructor<>())
            .LHMEMFN(finish)
            .LHMEMFN(kill)
            .LHPROPG(state)
            .LHISREFVALID
            .enum_("state") [
                value("CREATED", Entity::EntityState::created),
                value("FINISHED", Entity::EntityState::finished),
                value("KILLED", Entity::EntityState::killed)
            ]
            .def("components", &getComponents, dependency(_1, result))
            .def("component", &getComponent)
            .def("require", &requireComponent)
            .def(tostring(const_self))
            .def(const_self == other<Entity*>())
#       undef LHCURCLASS // avoid compiler warning
    ];
    lua_getglobal(vm.L(), "jd");
    luabind::object(vm.L(), static_cast<Entity*>(nullptr)).push(vm.L());
    lua_setfield(vm.L(), -2, "NIL_ENTITY");
    lua_pop(vm.L(), 1);
}
