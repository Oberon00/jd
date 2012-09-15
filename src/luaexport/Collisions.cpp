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

class LTileCollideableGroup: public TileCollideableGroup, public CollideableGroupRemover {
public:
    LTileCollideableGroup(CollisionManager& parent, std::string const& id, jd::Tilemap& tm):
        TileCollideableGroup(tm),
        CollideableGroupRemover(id, parent)
      { parent.addGroup(id, *this); }

private:
    LTileCollideableGroup operator=(LTileCollideableGroup const&);
};

class LRectCollideableGroup: public RectCollideableGroup, public CollideableGroupRemover {
public:
    LRectCollideableGroup(CollisionManager& parent, std::string const& id):
        CollideableGroupRemover(id, parent)
      { parent.addGroup(id, *this); }
private:
    LRectCollideableGroup operator=(LTileCollideableGroup const&);

};

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

#       define LHCURCLASS CollisionManager
        LHCLASS
            .def(constructor<>())
            .LHMEMFN(addPairing)
            .def("group", &LHCURCLASS::operator[])
            .LHMEMFN(collide)
            .def("colliding",
                (CollisionVec(LHCURCLASS::*)(sf::FloatRect const&))
                    &LHCURCLASS::colliding)
            .def("colliding",
                (CollisionVec(LHCURCLASS::*)(sf::FloatRect const&, std::string const&))
                    &LHCURCLASS::colliding),
#       undef LHCURCLASS

#       define LHCURCLASS CollideableGroup
        LHCLASS
            .def("colliding", &CollideableGroup_colliding)
            .def("colliding", 
                (CollisionVec const(LHCURCLASS::*)(sf::FloatRect const&, Entity* e))
                    &LHCURCLASS::colliding)
            .def("colliding", 
                (CollisionVec const(LHCURCLASS::*)(sf::Vector2f, sf::Vector2f))
                    &LHCURCLASS::colliding)
            .LHMEMFN(collideWith)
            .LHMEMFN(clear),
#       undef LHCURCLASS

#       define LHCURCLASS CollideableGroupRemover
        LHCLASS
            .LHPROPG(manager)
            .LHPROPG(id),
#       undef LHCURCLASS

        class_<TileCollideableGroup, CollideableGroup>("@TileCollideableGroup@"),

#       define LHCURCLASS LTileCollideableGroup
        class_<LHCURCLASS,
            bases<
                CollideableGroupRemover,
                TileCollideableGroup>
        >("TileCollideableGroup")
            .def(constructor<CollisionManager&, std::string const&, jd::Tilemap&>())
            .LHMEMFN(setProxy)
            .LHMEMFN(setColliding)
            .LHPROPG(tilemap),
#       undef LHCURCLASS

        class_<RectCollideableGroup, CollideableGroup>("@RectCollideableGroup@"),
#       define LHCURCLASS LRectCollideableGroup
        class_<LHCURCLASS,
            bases<
                CollideableGroupRemover,
                RectCollideableGroup>
        >("RectCollideableGroup")
            .def(constructor<CollisionManager&, std::string const&>())
            .LHMEMFN(add)
            .LHMEMFN(remove)
#       undef LHCURCLASS
    
    ];
}