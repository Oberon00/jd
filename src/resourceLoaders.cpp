#include "resourceLoaders.hpp"
#include "ResourceManager.hpp"
#include "exceptions.hpp"
#include <physfs.h>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Font.hpp>
#include "VFileFont.hpp"
#include <SFML/Audio/SoundBuffer.hpp>

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

LOAD_TRAITS(sf::Image,       "res/img/",  9, (".png", ".jpg", ".bmp", ".jpeg", ".tga", ".gif", ".psd", ".hdr", ".pic"));
LOAD_TRAITS(sf::Texture,     "res/img/",  9, (".png", ".jpg", ".bmp", ".jpeg", ".tga", ".gif", ".psd", ".hdr", ".pic"));
LOAD_TRAITS(VFileFont,       "res/fnt/",  2, (".ttf", ".otf"));
LOAD_TRAITS(sf::SoundBuffer, "res/snd/", 22, (".ogg", ".wav", ".flac", ".aiff", ".au", ".raw", ".paf", ".svx", ".nist",  \
                                              ".voc", ".ircam", ".w64", ".mat4", ".mat5" ".pvf", ".htk", ".sds", ".avr", \
                                              ".sd2", ".caf", ".wve", ".mpc2k", ".rf64"));

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

static void loadFontResource(VFileFont& fnt, std::string const& name)
{
    std::string const filename = findResource(
        name,
        ResLoadTraits<VFileFont>::prefix,
        &ResLoadTraits<VFileFont>::exts.front(),
        ResLoadTraits<VFileFont>::exts.size());
    fnt.stream.open(filename);
    if (!fnt.loadFromStream(fnt.stream)) {
        throw jd::ResourceLoadError(
            "failed loading resource \"" + name +
            "\" from file \"" + filename + "\"");
    }
}

static bool initSfResources()
{
    log(); // init logfile

    resMng<sf::Image>().setResourceNotFoundCallback(
        &loadSfmlResource<sf::Image>);
    resMng<sf::Texture>().setResourceNotFoundCallback(&loadTextureResource);

    resMng<VFileFont>().setResourceNotFoundCallback(&loadFontResource);
    resMng<sf::SoundBuffer>().setResourceNotFoundCallback(&loadSfmlResource<sf::SoundBuffer>);

    return bool();
}

static bool const dummy = initSfResources();
