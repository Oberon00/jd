#include "compsys/Entity.hpp"
#include "compsys/Component.hpp"
#include <luabind/adopt_policy.hpp>
#include "compsys/ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"
#include <boost/foreach.hpp>
#include "LuaUtils.hpp"
#include "container.hpp"

char const libname[] = "EntitySystem";
#include "ExportThis.hpp"
#include <luabind/operator.hpp>
#include <luabind/dependency_policy.hpp>
#include <luabind/iterator_policy.hpp>

static std::vector<luabind::object> getComponents(Entity& this_, lua_State* L)
{
    LUAU_BALANCED_STACK_DBG(L);
    auto const& comps = this_.components();
    std::vector<luabind::object> result;
    BOOST_FOREACH (Component const& c, comps) {
        c.metaComponent().castUp(L, const_cast<Component*>(&c));
        result.push_back(luabind::object(luabind::from_stack(L, -1)));
        lua_pop(L, 1);
    }
    return result;
}

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
    os << "jd.Entity ( @" << &e << "; ";
    auto const& comps = e.components();
    BOOST_FOREACH (Component const& c, comps)
        os << c.metaComponent().name();
    return os << ')';
}

static bool operator==(Entity const& lhs, Entity const* rhs)
    { return &lhs == rhs; }

void init(LuaVm& vm)
{
    typedef std::vector<luabind::object> ObjectVec;
    luabind::class_<ObjectVec> cObjectVec("ComponentList");
    exportRandomAccessContainer(cObjectVec);

    vm.initLib("ComponentSystem");
    LHMODULE [

#       define LHCURCLASS Entity
        class_<LHCURCLASS, WeakRef<LHCURCLASS>>("Entity")
            .scope [
                cObjectVec
            ]
            .def(constructor<>())
            .LHMEMFN(finish)
            .LHMEMFN(kill)
            .LHPROPG(state)
            .def("components", &getComponents, dependency(_1, result))
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
