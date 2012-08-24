#include "resourceLoaders.hpp"
#include "ResourceManager.hpp"
#include "exceptions.hpp"
#include <physfs.h>
#include "SFML/Graphics/Image.hpp"
#include "SFML/Graphics/Texture.hpp"

std::string const findResource(
    std::string const& name,
    std::string const& prefix,
    char const* const* exts,
    std::size_t nExts)
{
    if (PHYSFS_exists(name.c_str()))
        return name;

    std::string const path(prefix + name);
    if (PHYSFS_exists(path.c_str()))
        return path;

    if (!exts)
        throw jd::ResourceLoadError(
            "could not find \"" + name  + "\"; tried \"" +
            path + "\"; no extensions given");

    std::string result(path + ".???");
    for (std::size_t i = 0; i < nExts; ++i) {
        result.replace(result.begin() + path.size(), result.end(), exts[i]);
        if (PHYSFS_exists(result.c_str()))
            return result;
    }

    throw jd::ResourceLoadError(
            "could not find \"" + name  + "\"; tried \"" +
            path + "\" (also with usual extensions appended)");
}

LOAD_TRAITS(sf::Image,   "res/img/", 6, (".png", ".jpg", ".jpeg", ".gif", ".tga", ".bmp"));
LOAD_TRAITS(sf::Texture, "res/img/", 6, (".png", ".jpg", ".jpeg", ".gif", ".tga", ".bmp"));

static void loadTextureResource(sf::Texture& tx, std::string const& name)
{
    auto image = resMng<sf::Image>().tryGet(name);
    if (image) {
        if (!tx.loadFromImage(*image)) {
            throw jd::ResourceLoadError(
                "failed loading texture resource \"" +
                name + "\" from image.");
        }
    } else {
        loadSfmlResource(tx, name);
    }


}

static bool initSfResources()
{
    log(); // init logfile
    resMng<sf::Image>().setResourceNotFoundCallback(
        &loadSfmlResource<sf::Image>);

    resMng<sf::Texture>().setResourceNotFoundCallback(&loadTextureResource);

    return bool();
}

static bool const dummy = initSfResources();
