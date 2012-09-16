#include "Tilemap.hpp"
#include "Geometry.hpp"
#include "TransformGroup.hpp" // GroupedDrawable
#include "sharedPtrConverter.hpp"
#include "container.hpp"
#include "MapInfo.hpp"

static char const libname[] = "Tilemap";
#include "ExportThis.hpp"

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
    exportAssocContainer(cPropertyMap);

    class_<MapObjectGroup::Map> cObjectGroupMap("ObjectGroupTable");
    exportAssocContainer(cObjectGroupMap);

    class_<std::vector<PropertyMap>> cPropertyMapVec("StringTableList");
    exportRandomAccessContainer(cPropertyMapVec);

    class_<std::vector<sf::Vector2f>> cPointVec("PointList");
    exportRandomAccessContainer(cPointVec);

    class_<std::vector<MapObject>> cMapObjectVec("ObjectList");
    exportRandomAccessContainer(cMapObjectVec);
    
    LHMODULE [
        namespace_("mapInfo") [
            cPropertyMap,
            cObjectGroupMap,
            cPropertyMapVec,
            cMapObjectVec,
            cPointVec,

#           define LHCURCLASS MapInfo
            class_<LHCURCLASS>("Map")
                .LHPROPRW(tileProperties)
                .LHPROPRW(layerProperties)
                .LHPROPRW(objectGroups),
#           undef LHCURCLASS

#           define LHCURCLASS MapObject
            class_<LHCURCLASS>("Object")
                .LHPROPRW(name)
                .LHPROPRW(type)
                .LHPROPRW(position)
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
            .def(constructor<TransformGroup&>())
            .def("get", &Tilemap_get)
            .def("set", &Tilemap_set)
            .property("bounds", &LHCURCLASS::getGlobalBounds)
            .property("localBounds", &LHCURCLASS::getLocalBounds)
            .property("tileset", &LHCURCLASS::tileset, &LHCURCLASS::setTileset)
            .property("size", &LHCURCLASS::size, &LHCURCLASS::setSize)
            .LHMEMFN(localTilePos)
            .def("tilePos", &LHCURCLASS::globalTilePos)
            .LHMEMFN(tilePosFromLocal)
            .LHMEMFN(tilePosFromGlobal)
            .def("loadFromFile", &loadTilemap)
#   undef LHCURCLASS
    ];
}