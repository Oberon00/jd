#include "Collisions.hpp"
#include "Geometry.hpp"
#include "compsys/Entity.hpp"
#include "TileCollideableGroup.hpp"
#include "RectCollideableGroup.hpp"
#include "container.hpp"
#include "svc/ServiceLocator.hpp"
#include "Tilemap.hpp"

static char const libname[] = "Collisions";
#include "ExportThis.hpp"

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

} // anonymous namespace

static void init(LuaVm& vm)
{
    typedef std::vector<Collision> CollisionVec;
    luabind::class_<CollisionVec> cCollisionVec("CollisionList");
    exportRandomAccessContainer(cCollisionVec);
    LHMODULE [
#       define LHCURCLASS Collision
        LHCLASS
            .property("entity", &Collision_getEntity, &Collision_setEntity)
            .LHPROPRW(rect),
#       undef LHCURCLASS
        cCollisionVec,

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

#       define LHCURCLASS TileCollideableGroup
        class_<LHCURCLASS, CollideableGroup>("TileCollideableGroup")
            .def(constructor<jd::Tilemap&>())
            .LHMEMFN(setProxy)
            .LHMEMFN(setColliding)
            .LHPROPG(tilemap),
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