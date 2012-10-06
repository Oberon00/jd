#include "Collisions.hpp"
#include "Geometry.hpp"
#include "compsys/Entity.hpp"
#include "TileCollideableGroup.hpp"
#include "RectCollideableGroup.hpp"
#include "comp/TileCollisionComponent.hpp"
#include "container.hpp"
#include "svc/ServiceLocator.hpp"
#include "Tilemap.hpp"
#include "LuaFunction.hpp"

static char const libname[] = "Collisions";
#include "ExportThis.hpp"
#include <luabind/out_value_policy.hpp>


namespace {

static WeakRef<Entity> Collision_getEntity(Collision const& c)
{
    return c.entity->ref();
}

static void Collision_setEntity(Collision& c, WeakRef<Entity> e)
{
    c.entity = e.getOpt();
}


static std::vector<Collision> const CollideableGroup_colliding(
    CollideableGroup& self, sf::FloatRect const& r)
{
    return self.colliding(r);
}

static void TileStackCollideableGroup_callFilter(
    luabind::object& filter,
    sf::Vector2u pos,
    std::vector<TileStackCollideableGroup::Info>& stack)
{
    filter(pos, &stack);
}


static void TileStackCollideableGroup_setFilter(
    TileStackCollideableGroup& this_, luabind::object filter)
{
    this_.setFilter(boost::bind(
        &TileStackCollideableGroup_callFilter, filter, _1, _2));
}

static void TileStackCollideableGroup_Info_setEntity(
    TileStackCollideableGroup::Info& this_, WeakRef<Entity> e)
{
    this_.entity = e;
}

static luabind::object TileStackCollideableGroup_Info_entity(
    TileStackCollideableGroup::Info const& this_, lua_State* L)
{
    if (!this_.entity)
        return luabind::object();
    return luabind::object(L, this_.entity);
}



} // anonymous namespace

static void init(LuaVm& vm)
{
    typedef std::vector<Collision> CollisionVec;
    luabind::class_<CollisionVec> cCollisionVec("CollisionList");
    exportRandomAccessContainer<true>(cCollisionVec);

    typedef std::vector<Vector3u> PositionVec;
    luabind::class_<PositionVec> cPositionVec("PositionList");
    exportRandomAccessContainer<false>(cPositionVec);

    typedef std::vector<TileStackCollideableGroup::Info> TileStackInfoVec;
    luabind::class_<TileStackInfoVec> cTileStackInfoVec("TileStackCollisionVec");
    exportRandomAccessContainer<true>(cTileStackInfoVec);

    LHMODULE [
#       define LHCURCLASS Collision
        LHCLASS
            .property("entity", &Collision_getEntity, &Collision_setEntity)
            .LHPROPRW(rect),
#       undef LHCURCLASS
        cCollisionVec,
        cPositionVec,
#       define LHCURCLASS CollideableGroup
        LHCLASS
            .def("colliding", &CollideableGroup_colliding)
            .def("colliding", 
                (CollisionVec (LHCURCLASS::*)(sf::FloatRect const&, Entity* e))
                    &LHCURCLASS::colliding)
            .def("colliding", 
                (CollisionVec (LHCURCLASS::*)(sf::Vector2f, sf::Vector2f))
                    &LHCURCLASS::colliding)
            .LHMEMFN(collideWith)
            .LHMEMFN(collide)
            .LHMEMFN(clear),
#       undef LHCURCLASS

#       define LHCURCLASS TileCollideableInfo
        LHCLASS
            .def(constructor<jd::Tilemap&>())
            .LHMEMFN(setProxy)
            .LHMEMFN(proxy)
            .LHMEMFN(setColliding)
            .def("colliding", 
                (CollisionVec (LHCURCLASS::*)(sf::FloatRect const&, Entity* e, PositionVec*))
                    &LHCURCLASS::colliding, pure_out_value(_4))
            .def("colliding", 
                (CollisionVec (LHCURCLASS::*)(sf::Vector2f, sf::Vector2f, PositionVec*))
                    &LHCURCLASS::colliding, pure_out_value(_4))
            .def("colliding", 
                (TileCollisionComponent* (LHCURCLASS::*)(Vector3u))&LHCURCLASS::colliding)
            .LHPROPG(tilemap),
#       undef LHCURCLASS

#       define LHCURCLASS TileLayersCollideableGroup
        class_<LHCURCLASS, CollideableGroup>("TileLayersCollideableGroup")
            .def(constructor<TileCollideableInfo*, unsigned, unsigned>())
            .property("firstLayer", &LHCURCLASS::firstLayer, &LHCURCLASS::setFirstLayer)
            .property("endLayer", &LHCURCLASS::endLayer, &LHCURCLASS::setEndLayer)
            .LHPROPG(data),
#       undef LHCURCLASS

#       define LHCURCLASS TileStackCollideableGroup
        class_<LHCURCLASS, CollideableGroup>("TileStackCollideableGroup")
            .def(constructor<TileCollideableInfo*>())
            .def("setFilter", &TileStackCollideableGroup_setFilter)
            .LHPROPG(data)
            .scope [
                cTileStackInfoVec,
#               undef LHCURCLASS // keep Visual Studio happy
#               define LHCURCLASS TileStackCollideableGroup::Info
                class_<LHCURCLASS>("TileStackCollideableInfo")
                    .def(constructor<>())
                    .def(constructor<unsigned, WeakRef<Entity> const&>())
                    .LHPROPRW(tileId)
                    .property("entity",
                        &TileStackCollideableGroup_Info_entity,
                        &TileStackCollideableGroup_Info_setEntity)
                    .LHPROPRW(discard)
            ],
#       undef LHCURCLASS

#       define LHCURCLASS RectCollideableGroup
        class_<LHCURCLASS, CollideableGroup>("RectCollideableGroup")
            .def(constructor<>())
            .LHMEMFN(add)
            .LHMEMFN(remove),
#       undef LHCURCLASS

#       define LHCURCLASS CollideableGroupGroup
        class_<LHCURCLASS, CollideableGroup>("CollideableGroupGroup")
            .def(constructor<>())
            .LHMEMFN(add)
            .LHMEMFN(remove)
#       undef LHCURCLASS

    ];
}