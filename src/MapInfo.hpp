#ifndef MAP_INFO_HPP_INCLUDED
#define MAP_INFO_HPP_INCLUDED MAP_INFO_HPP_INCLUDED

#include "Tilemap.hpp"

#include <SFML/System/Vector2.hpp>

#include <string>
#include <unordered_map>
#include <vector>


typedef std::unordered_map<std::string, std::string> PropertyMap;

struct MapObject {
    enum T { rect, tile, line, poly };
    int objectType;
    PropertyMap properties;
    std::string name;
    std::string type;
    sf::Vector2f position;
    sf::Vector2f size;
    unsigned tileId;
    std::vector<sf::Vector2f> relativePoints;
    std::vector<sf::Vector2f> absolutePoints() const;
};

struct MapObjectGroup {
    std::string name;
    PropertyMap properties;
    std::vector<MapObject> objects;
    typedef std::unordered_map<std::string, MapObjectGroup> Map;
};



struct MapInfo {
    PropertyMap mapProperties;
    std::vector<PropertyMap> tileProperties;

    // Note: an extra property "name" will be stored containing the layers name attribute
    std::vector<PropertyMap> layerProperties;
    MapObjectGroup::Map objectGroups;
};

MapInfo loadTilemap(jd::Tilemap& tm, std::string const& vfilename);
std::vector<PropertyMap> loadTileset(jd::Tileset& ts, std::string const& vfilename);

#endif
