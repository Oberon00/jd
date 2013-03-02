#include "Collisions.hpp"

#include "comp/TileCollisionComponent.hpp"
#include "compsys/Entity.hpp"
#include "container.hpp"
#include "LuaFunction.hpp"
#include "RectCollideableGroup.hpp"
#include "SfBaseTypes.hpp"
#include "sfUtil.hpp"
#include "svc/ServiceLocator.hpp"
#include "TileCollideableGroup.hpp"
#include "Tilemap.hpp"

#include <luabind/out_value_policy.hpp>

#include <algorithm>

static char const libname[] = "Collisions";
#include "ExportThis.hpp"


static luabind::object TileCollideableInfo_colliding(
    lua_State* L, TileCollideableInfo& this_, Vector3u pos)
{
    auto c = this_.colliding(pos);
    return c.valid() ? luabind::object(L, c) : luabind::object();

}

static luabind::object TileCollideableInfo_proxy(
    lua_State* L, TileCollideableInfo& this_, unsigned tid)
{
    auto p = this_.proxy(tid);
    return p.valid() ? luabind::object(L, p) : luabind::object();
}


static luabind::object Collision_getEntity(lua_State* L, Collision const& c)
{
    return c.entity ?
        luabind::object(L, c.entity->ref()) : luabind::object();
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

static void CollisionVec_sortByDistance(
    std::vector<Collision>& this_, sf::Vector2f to)
{
    std::sort(
        begin(this_), end(this_),
        [to](Collision const& c1, Collision const& c2) -> bool {
            auto const c1next = jd::nearestPoint(c1.rect, to);
            auto const c2next = jd::nearestPoint(c2.rect, to);
            auto const c1d = jd::manhattanDistance(c1next, to);
            auto const c2d = jd::manhattanDistance(c2next, to);
            return c1d < c2d;
    });
}

static void CollisionVec_differenceTo(
    std::vector<Collision> cv1, std::vector<Collision> cv2,
    std::vector<Collision>& only1, std::vector<Collision>& only2)
{
    static auto const collisionLess = [](
        Collision const& c1, Collision const& c2
    ) -> bool {
        auto const& r1 = c1.rect;
        auto const& r2 = c2.rect;
        return r1.left == r2.left ?
                    r1.top == r2.top ?
                        r1.width == r2.width ?
                            r1.height < r2.height :
                        r1.width < r2.width :
                    r1.top < r2.top :
               r1.left < r2.left;
    };
    only1.reserve(cv1.size());
    only2.reserve(cv2.size());
    std::sort(cv1.begin(), cv1.end(), collisionLess);
    std::sort(cv2.begin(), cv2.end(), collisionLess);
    std::set_difference(
        cv1.begin(), cv1.end(), cv2.begin(), cv2.end(),
        std::back_inserter(only1), collisionLess);
     std::set_difference(
        cv2.begin(), cv2.end(), cv1.begin(), cv1.end(),
        std::back_inserter(only2), collisionLess);
}

static void init(LuaVm& vm)
{
    using namespace luabind;
    typedef std::vector<Collision> CollisionVec;
    class_<CollisionVec> cCollisionVec("CollisionList");
    exportRandomAccessContainer<true>(cCollisionVec);
    cCollisionVec
        .def("sortByDistance", &CollisionVec_sortByDistance)
        .def("differenceTo", &CollisionVec_differenceTo,
            pure_out_value(_3) + pure_out_value(_4));

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
            .def("proxy", &TileCollideableInfo_proxy)
            .LHMEMFN(setColliding)
            .def("colliding", 
                (CollisionVec (LHCURCLASS::*)(sf::FloatRect const&, Entity* e, PositionVec*))
                    &LHCURCLASS::colliding, pure_out_value(_4))
            .def("colliding", 
                (CollisionVec (LHCURCLASS::*)(sf::Vector2f, sf::Vector2f, PositionVec*))
                    &LHCURCLASS::colliding, pure_out_value(_4))
            .def("colliding", &TileCollideableInfo_colliding)
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
