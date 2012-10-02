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

static void TileStackCollideableGroup_setFilter(
    TileStackCollideableGroup& this_, luabind::object filter)
{
    this_.setFilter(LuaFunction<void>(filter));
}

} // anonymous namespace

static void init(LuaVm& vm)
{
    typedef std::vector<Collision> CollisionVec;
    typedef std::vector<Vector3u> PositionVec;
    luabind::class_<CollisionVec> cCollisionVec("CollisionList");
    exportRandomAccessContainer(cCollisionVec);

    luabind::class_<PositionVec> cPositionVec("PositionList");
    exportRandomAccessContainer(cPositionVec);
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
#               undef LHCURCLASS // keep Visual Studio happy
#               define LHCURCLASS TileStackCollideableGroup::Info
                LHCLASS
                    .def(constructor<>())
                    .def(constructor<unsigned, WeakRef<Entity> const&>())
                    .LHPROPRW(tileId)
                    .LHPROPRW(entity)
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