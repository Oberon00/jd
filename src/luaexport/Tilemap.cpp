#include "Tilemap.hpp"
#include "TransformGroup.hpp" // GroupedDrawable
#include "sharedPtrConverter.hpp"
#include "container.hpp"
#include "MapInfo.hpp"

static char const libname[] = "Tilemap";
#include "ExportThis.hpp"
#include "Geometry.hpp"

static unsigned Tilemap_get(jd::Tilemap const& map, sf::Vector3i p)
{
    if (!map.isValidPosition(p))
        throw "invalid tile position (@get)";
    return map[static_cast<jd::Vector3u>(p)];
}

static void Tilemap_set(jd::Tilemap& map, sf::Vector3i p, unsigned tid)
{
    if (!map.isValidPosition(p))
        throw "invalid tile position (@set)";
    map.set(static_cast<jd::Vector3u>(p), tid);
}

static void init(LuaVm& vm)
{
    vm.initLib("SfGraphics");

    using namespace luabind;
    class_<PropertyMap> cPropertyMap("StringTable");
    exportAssocContainer<false>(cPropertyMap);

    class_<MapObjectGroup::Map> cObjectGroupMap("ObjectGroupTable");
    exportAssocContainer<true>(cObjectGroupMap);

    class_<std::vector<PropertyMap>> cPropertyMapVec("StringTableList");
    exportRandomAccessContainer<true>(cPropertyMapVec);

    class_<std::vector<sf::Vector2f>> cPointVec("PointList");
    exportRandomAccessContainer<false>(cPointVec);

    class_<std::vector<MapObject>> cMapObjectVec("ObjectList");
    exportRandomAccessContainer<true>(cMapObjectVec);
    
    LHMODULE [
        namespace_("mapInfo") [
            cPropertyMap,
            cObjectGroupMap,
            cPropertyMapVec,
            cMapObjectVec,
            cPointVec,

#           define LHCURCLASS MapInfo
            class_<LHCURCLASS>("Map")
                .def(constructor<LHCURCLASS const&>())
                .LHPROPRW(tileProperties)
                .LHPROPRW(layerProperties)
                .LHPROPRW(objectGroups),
#           undef LHCURCLASS

#           define LHCURCLASS MapObject
            class_<LHCURCLASS>("Object")
                .def(constructor<LHCURCLASS const&>())
                .LHPROPRW(name)
                .LHPROPRW(type)
                .LHPROPRW(position)
                .LHPROPRW(size)
                .LHPROPRW(tileId)
                .LHPROPRW(objectType)
                .LHPROPRW(relativePoints)
                .LHPROPG(absolutePoints)
                .enum_("t")[
                    value("RECT", MapObject::rect),
                    value("TILE", MapObject::tile),
                    value("LINE", MapObject::line),
                    value("POLY", MapObject::poly)
                ]
                .LHPROPRW(properties),
#           undef LHCURCLASS

#           define LHCURCLASS MapObjectGroup
            class_<LHCURCLASS>("ObjectGroup")
                .def(constructor<LHCURCLASS const&>())
                .LHPROPRW(name)
                .LHPROPRW(objects)
                .LHPROPRW(properties)
#           undef LHCURCLASS

        ],

#   define LHCURCLASS jd::Tileset
        class_<LHCURCLASS>("Tileset")
            .def(constructor<
                sf::Vector2u,
                // HACK: For some reason, ConstPtr is not recognized by luabind:
                ResourceTraits<sf::Texture>::Ptr>())
            .def("texturePosition", &LHCURCLASS::position)
            .LHPROPG(size)
            .LHPROPG(texture),
#   undef LHCURCLASS
        class_<jd::Tilemap, bases<sf::Drawable, sf::Transformable>>("@Tilemap@"),
#   define LHCURCLASS GroupedDrawable<jd::Tilemap>
		class_<LHCURCLASS, bases<TransformGroup::AutoEntry, jd::Tilemap>>("Tilemap")
            .def(constructor<>())
            .def(constructor<TransformGroup&>())
            .def(constructor<LHCURCLASS const&>())
            .property("group", &LHCURCLASS::group, &LHCURCLASS::setGroup)
            .LHMEMFN(isValidPosition)
            .def("get", &Tilemap_get)
            .def("set", &Tilemap_set)
            .property("bounds", &LHCURCLASS::getGlobalBounds)
            .property("localBounds", &LHCURCLASS::getLocalBounds)
            .property("tileset", &LHCURCLASS::tileset, &LHCURCLASS::setTileset)
            .property("size", &LHCURCLASS::size, &LHCURCLASS::setSize)
            .LHMEMFN(localTilePos)
            .def("addAnimation",
                (void(LHCURCLASS::*)(std::size_t, std::size_t, float))
                &LHCURCLASS::addAnimation)
            .def("addAnimation",
                (void(LHCURCLASS::*)(jd::Vector3u, std::size_t, float))
                &LHCURCLASS::addAnimation)
            .def("removeAnimation",
                (void(LHCURCLASS::*)(std::size_t))&LHCURCLASS::removeAnimation)
            .def("removeAnimation",
                (void(LHCURCLASS::*)(jd::Vector3u))&LHCURCLASS::removeAnimation)
            .LHMEMFN(animate)
            .def("tilePos", &LHCURCLASS::globalTilePos)
            .LHMEMFN(tilePosFromLocal)
            .LHMEMFN(tilePosFromGlobal)
            .def("tileRect", &LHCURCLASS::globalTileRect)
            .LHMEMFN(localTileRect)
            .def("loadFromFile", &loadTilemap)
#   undef LHCURCLASS
    ];
}