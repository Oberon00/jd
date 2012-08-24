#include "compsys/Entity.hpp"
#include "compsys/EntityCollection.hpp"
#include <luabind/adopt_policy.hpp>
#include "compsys/ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"

char const libname[] = "EntitySystem";
#include "ExportThis.hpp"
#include <luabind/operator.hpp>

// Entities and EntityCollections cannot created with createToplevel() must
// really stay toplevel, because they are owned and garbage-collected by Lua.
// Although the API does not offer a method to add them to EntityCollections,
// the dummyParent enforces this again, so it cannot be forgotten.
static EntityCollection dummyParent;

//static Entity* createEntity(std::string const& id) { return new Entity(id); }
static Entity* createAnonymousEntity() { return new Entity(dummyParent); }
//static EntityCollection* createEntityCollection(std::string const& id)
//{
//    return new EntityCollection(id);
//}
static EntityCollection* createAnonymousEntityCollection()
{
    return new EntityCollection(dummyParent);
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

static luabind::object getEntity(EntityCollection& this_, lua_State* L, std::string const& id)
{
    
    if (Entity* e = this_[id])
        return luabind::object(L, e->ref());
    return luabind::object();
}

static luabind::object getEntityCollection(EntityCollection& this_, lua_State* L, std::string const& id)
{
    
    if (EntityCollection* ec = this_.collection(id))
        return luabind::object(L, ec->ref());
    return luabind::object();
}

static luabind::object getEntityParent(Entity& this_, lua_State* L)
{
    if (EntityCollection* ec = this_.parent())
        return luabind::object(L, ec->ref());
    return luabind::object();
}

static luabind::object getEntityCollectionParent(EntityCollection& this_, lua_State* L)
{
    if (EntityCollection* ec = this_.parent())
        return luabind::object(L, ec->ref());
    return luabind::object();
}

static std::ostream& operator<< (std::ostream& os, Entity const& e)
{
    os << "jd.Entity (";
    if (!e.id().empty())
        os << '#' << e.id() << " @";
    return os << &e << ')';
}

static std::ostream& operator<< (std::ostream& os, EntityCollection const& e)
{
    os << "jd.EntityCollection (";
    if (!e.id().empty())
        os << '#' << e.id() << " @";
    return os << &e << ')';
}

static bool operator==(Entity const& lhs, Entity const* rhs)
    { return &lhs == rhs; }
static bool operator==(EntityCollection const& lhs, EntityCollection const* rhs)
    { return &lhs == rhs; }


void init(LuaVm& vm)
{
    vm.initLib("ComponentSystem");
    LHMODULE [

#       define LHCURCLASS Entity
        class_<LHCURCLASS, WeakRef<LHCURCLASS>>("Entity")
            .def(constructor<EntityCollection&>())
            .def(constructor<EntityCollection&, std::string const&>())
            .scope [
                def("createToplevel", &createAnonymousEntity, adopt(result))
            ]
            .LHMEMFN(finish)
            .LHMEMFN(kill)
            .LHPROPG(state)
            .property("id", &LHCURCLASS::id, &LHCURCLASS::setId)
            .property("parent", &getEntityParent)
            .def("component", &getComponent)
            .def("require", &requireComponent)
            .def(tostring(const_self))
            .def(const_self == other<Entity*>()),
#       undef LHCURCLASS // avoid compiler warning

#       define LHCURCLASS EntityCollection
        class_<LHCURCLASS, WeakRef<LHCURCLASS>>("EntityCollection")
            .def(constructor<EntityCollection&>())
            .def(constructor<EntityCollection&, std::string const&>())
            .scope [
                def("createToplevel", &createAnonymousEntityCollection, adopt(result))
            ]
            .LHMEMFN(clear)
            .LHMEMFN(tidy)
            .LHMEMFN(kill)
            .LHPROPG(killed)
            .property("id", &LHCURCLASS::id, &LHCURCLASS::setId)
            .property("parent", &getEntityCollectionParent)
            .def("entity", &getEntity)
            .def("collection", &getEntityCollection)
            .def(tostring(const_self))
            .def(const_self == other<EntityCollection*>())
#       undef LHCURCLASS
    ];

    //lua_State* L = vm.L();
    //lua_getglobal(L, "jd");
    //lua_getfield(L, -1, "Entity");
    //lua_getmetatable(L, -1
}
