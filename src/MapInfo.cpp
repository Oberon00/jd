// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "MapInfo.hpp"

#include "base64.hpp"
#include "Logfile.hpp"
#include "ressys/ResourceManager.hpp"
#include "svc/FileSystem.hpp"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/format.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/range/algorithm/count_if.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/range/algorithm/transform.hpp>
#include <SFML/Graphics/Image.hpp>
#include <zlib.h>

#include <sstream>


namespace io = boost::iostreams;
namespace pt = boost::property_tree;
namespace rng = boost::range;
using boost::format;
using boost::lexical_cast;

std::vector<sf::Vector2f> MapObject::absolutePoints() const
{
    std::vector<sf::Vector2f> result(relativePoints.size());
    std::transform(
        relativePoints.begin(), relativePoints.end(),
        result.begin(),
        [this](sf::Vector2f p) { return p += position; });
    return result;
}

static sf::Color colorFromHexString(std::string const& s)
{
    if (s.empty())
        throw std::runtime_error("empty hex-color-string");
    std::istringstream oss(s.front() == '#' ? s.substr(1) : s);

    unsigned sval;
    oss >> std::hex >> sval;
    if (!oss)
        throw std::runtime_error("could not convert hex-color-string \"" + s + "\" to color");

    return sf::Color(
        static_cast<sf::Uint8>(sval & 0x00FF0000u) >> '\4',
        static_cast<sf::Uint8>(sval & 0x0000FF00u) >> '\1',
        static_cast<sf::Uint8>(sval & 0x000000FFu),
        static_cast<sf::Uint8>(sval & 0xFF000000u) >> '\6');
}

// TODO: information duplication: ResourceLoaders.cpp
static std::string correctPath(std::string p)
{
    std::string::size_type pos = p.find("res/img/") != std::string::npos ?
        p.find("res/img/") : p.find("\\res\\img\\");
    if (pos != std::string::npos)
        p = p.substr(pos + sizeof("res?img"));
    return p;
}

static pt::ptree readXmlVFile(std::string const& vfilename)
{
     // avoid most vexing parse by an additional pair of parantheses
    VFile f(vfilename);
    io::stream<VFileDevice> in(f);

    pt::ptree result;
    pt::read_xml(in, result);
    return result;
}

static ResourceTraits<sf::Texture>::Ptr loadTexture(pt::ptree const& dom, sf::Vector2u& imgsz)
{
    std::string const imgSource = correctPath(
       dom.get<std::string>("<xmlattr>.source"));
    auto img = resMng<sf::Image>().request(imgSource);
    imgsz = img->getSize();
    auto transColor = dom.get_optional<std::string>("<xmlattr>.trans");
    if (transColor)
        img->createMaskFromColor(colorFromHexString(*transColor));
    return resMng<sf::Texture>().request(imgSource);
}

static PropertyMap loadProperties(pt::ptree const& dom)
{
    PropertyMap result;
    for (auto const& propelem : dom) {
        if (propelem.first != "property") {
            LOG_W("unknown tag in properties: \"" + propelem.first + '\"');
            continue;
        }
        result[propelem.second.get<std::string>("<xmlattr>.name")] =
            propelem.second.get<std::string>("<xmlattr>.value");
    }
    return result;
}

static PropertyMap loadPropertiesOptional(pt::ptree const& parentdom)
{
    if (auto const props = parentdom.get_child_optional("properties"))
        return loadProperties(*props);
    return PropertyMap();
}

static std::vector<PropertyMap> loadTileset(
    jd::Tileset& ts,
    pt::ptree const& dom,
    std::string const& setdir)
{
    auto const src = dom.get_optional<std::string>("<xmlattr>.source");
    if (src)
        return loadTileset(ts, setdir + *src);

    sf::Vector2u imgsz;
    ts = jd::Tileset(
        sf::Vector2u(
            dom.get<unsigned>("<xmlattr>.tilewidth"),
            dom.get<unsigned>("<xmlattr>.tileheight")),
        loadTexture(dom.get_child("image"), imgsz));

    std::vector<PropertyMap> result(
        (imgsz.x / ts.size().x) * (imgsz.y / ts.size().y));

    for (auto const& tileelem : dom) {
        if (tileelem.first != "tile")
            continue;

        unsigned const tileId = tileelem.second.get<unsigned>("<xmlattr>.id");
        if (tileId >= result.size()) {
            LOG_W(format("tile id too high: %1%") % tileId);
            continue;
        }

        auto const& tileprops = tileelem.second.get_child_optional("properties");
        if (!tileprops)
            continue;
        result[tileId] = loadProperties(*tileprops);
        if (result[tileId].empty())
            LOG_W(format("empty tile properties for tile#%1%") % tileId);
    }
    return result;
}

std::vector<PropertyMap> loadTileset(jd::Tileset& ts, std::string const& vfilename)
{
    return loadTileset(ts, readXmlVFile(vfilename).get_child("tileset"), std::string());
}

static std::vector<sf::Vector2f> loadPointsOptional(pt::ptree const& parentdom)
{
    auto const points = parentdom.get_optional<std::string>("<xmlattr>.points");
    if (!points)
        return std::vector<sf::Vector2f>();

    typedef boost::iterator_range<std::string::const_iterator> SRange;
    std::vector<SRange> coordinates;
    boost::algorithm::split(coordinates, *points, boost::algorithm::is_any_of(" ,"));
    if (coordinates.size() % 2 != 0) {
        LOG_W(format(
            "Coordinate missing for point. Ignoring last coordinate (%1%).") %
                coordinates.back());
        coordinates.pop_back();
    }
    std::vector<sf::Vector2f> result;
    result.reserve(coordinates.size() / 2);
    try {
        for (auto it = coordinates.begin(); it != coordinates.end(); it += 2) {
            result.push_back(sf::Vector2f(
                lexical_cast<float>(*it),
                lexical_cast<float>(*(it + 1))));
        }
    } catch (boost::bad_lexical_cast const&) {
        LOG_W("Error loading points:"
              " coordinate string is not a valid point number floating");
        return std::vector<sf::Vector2f>();
    }
    return result;
}


MapInfo loadTilemap(jd::Tilemap& tm, std::string const& vfilename)
{
    auto const map = readXmlVFile(vfilename).get_child("map");

    tm.setSize(jd::Vector3u(
        map.get<unsigned>("<xmlattr>.width"),
        map.get<unsigned>("<xmlattr>.height"),
        static_cast<unsigned>(
            rng::count_if(map, [](pt::ptree::value_type const& e) {
                return e.first == "layer"; }))));
    if (tm.size().x * tm.size().y * tm.size().z == 0)
        throw jd::ResourceLoadError("map is empty");

    std::size_t const posSep = vfilename.find_first_of("/\\");
    jd::Tileset ts;
    MapInfo result;
    result.tileProperties = loadTileset(
        ts,
        map.get_child("tileset"),
        posSep == std::string::npos ?
            std::string() : vfilename.substr(0, posSep + 1));
    tm.setTileset(ts);

    result.layerProperties.resize(tm.size().z);

    unsigned z = 0;
    for (auto const& layer : map) {
        if (layer.first == "objectgroup") {
            std::string const groupName =
                layer.second.get<std::string>("<xmlattr>.name");
            MapObjectGroup& group = result.objectGroups[groupName];
            group.name = groupName;
            for (auto const& obj : layer.second) {
                if (obj.first == "properties") {
                    group.properties = loadProperties(obj.second);
                } else if (obj.first == "object") {
                    MapObject o;
                    o.name = obj.second.get("<xmlattr>.name", std::string());
                    o.type = obj.second.get("<xmlattr>.type", std::string());
                    o.position = sf::Vector2f(
                        obj.second.get<float>("<xmlattr>.x"),
                        obj.second.get<float>("<xmlattr>.y"));
                    o.size.x = obj.second.get("<xmlattr>.width", 0.f);
                    o.size.y = obj.second.get("<xmlattr>.height", 0.f);
                    o.properties = loadPropertiesOptional(obj.second);
                    o.tileId = obj.second.get("<xmlattr>.gid", 0u);
                    if (o.tileId) {
                        o.objectType = MapObject::T::tile;
                        o.position.y -= ts.size().y; // assuming map orientation is orthogonal.
                    } else if (auto const line = obj.second.get_child_optional("polyline")) {
                        o.objectType = MapObject::T::line;
                        o.relativePoints = loadPointsOptional(*line);
                    } else if (auto const line = obj.second.get_child_optional("polygon")) {
                        o.objectType = MapObject::T::poly;
                        o.relativePoints = loadPointsOptional(*line);
                    } else {
                        o.objectType = MapObject::T::rect; // WARN could also be invalid
                    }
                    group.objects.push_back(std::move(o));
                } else if (obj.first != "<xmlattr>") {
                    LOG_W("unknown tag in object group: \"" + obj.first + "\"");
                }
            }
        } else if (layer.first == "layer") {
            result.layerProperties[z]["name"] =
                layer.second.get("<xmlattr>.name", std::string());
            result.layerProperties[z] = loadPropertiesOptional(layer.second);
            pt::ptree const& datadom = layer.second.get_child("data");
            std::string const encoding =
                datadom.get<std::string>("<xmlattr>.encoding");
            if (encoding != "base64")
                throw jd::ResourceLoadError(str(format(
                    "layer#%1%: encoding \"%2%\" not supported") %
                        z % encoding));
            std::vector<base64::byte> rawdata = base64::decode(
                boost::algorithm::trim_copy(datadom.data()));
            if (rawdata.empty())
                throw jd::ResourceLoadError(str(format(
                    "layer#%1%: empty data") % z));
            std::string const compression =
                datadom.get("<xmlattr>.compression", std::string());
            std::vector<unsigned> data;
            if (compression.empty()) {
                data.assign(
                    reinterpret_cast<unsigned*>(&data.front()),
                    reinterpret_cast<unsigned*>(&data.back()));
            } else if (compression == "zlib" || compression == "gzip") {
                data.resize(tm.size().x * tm.size().y);
                uLongf bufsz = data.size() * sizeof(unsigned);
                int const r = uncompress(
                    reinterpret_cast<Bytef*>(&data.front()),
                    &bufsz,
                    reinterpret_cast<Bytef*>(&rawdata.front()),
                    rawdata.size());
                if (r != Z_OK)
                     throw jd::ResourceLoadError(str(format(
                        "layer#%1%: decompression failed: %2%") %
                        z % zError(r)));
                if (bufsz != data.size() * sizeof(unsigned))
                    throw jd::ResourceLoadError(str(format(
                        "layer#%1%: too less data") % z));
            } else {
                throw jd::ResourceLoadError(str(format(
                    "layer#%1%: compression \"%2%\" not supported") %
                     z % compression));
            }
            jd::Vector3u p(0, 0, z);
            std::size_t iData = 0;
            for (; p.y < tm.size().y; ++p.y) {
                for (; p.x < tm.size().x; ++p.x) {
                    tm.set(p, data[iData++]);
                }
                p.x = 0;
            }
            p.y = 0;
            ++z;
        }
    }
    return result;
}
