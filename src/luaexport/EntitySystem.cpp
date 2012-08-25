#include "compsys/Entity.hpp"
#include <luabind/adopt_policy.hpp>
#include "compsys/ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"

char const libname[] = "EntitySystem";
#include "ExportThis.hpp"
#include <luabind/operator.hpp>


static luabind::object getComponent(Entity& this_, lua_State* L, std::string const& name)
{
    Component* c = this_[ComponentRegistry::metaComponent(name)];
    if (!c)
        return luabind::object();
    c->metaComponent().castUp(L, c);
    return luabind::object(luabind::from_stack(L, -1));
}

static luabind::object requireComponent(Entity& this_, lua_State* L, std::string const& name)
{
    Component* c = this_[ComponentRegistry::metaComponent(name)];
    if (!c) {
        throw std::runtime_error(
            "required component \"" + name + "\" not available!");
    }
    c->metaComponent().castUp(L, c);
    return luabind::object(luabind::from_stack(L, -1));
}

static std::ostream& operator<< (std::ostream& os, Entity const& e)
{
    return os << "jd.Entity (" << &e << ')';
}

static bool operator==(Entity const& lhs, Entity const* rhs)
    { return &lhs == rhs; }

void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [

#       define LHCURCLASS Entity
        class_<LHCURCLASS, WeakRef<LHCURCLASS>>("Entity")
            .def(constructor<>())
            .LHMEMFN(finish)
            .LHMEMFN(kill)
            .LHPROPG(state)
            .def("component", &getComponent)
            .def("require", &requireComponent)
            .def(tostring(const_self))
            .def(const_self == other<Entity*>())
#       undef LHCURCLASS // avoid compiler warning
    ];

    //lua_State* L = vm.L();
    //lua_getglobal(L, "jd");
    //lua_getfield(L, -1, "Entity");
    //lua_getmetatable(L, -1
}
