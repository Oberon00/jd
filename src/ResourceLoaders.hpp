#ifndef RESOURCE_LOADERS_HPP_INCLUDED
#define RESOURCE_LOADERS_HPP_INCLUDED RESOURCE_LOADERS_HPP_INCLUDED

#include "svc/FileSystem.hpp"

#include <array>
#include <string>


template <typename ResT>
struct ResLoadTraits;

#define LOAD_TRAITS_DETAIL_EXPAND_ARGS(...) __VA_ARGS__

#define LOAD_TRAITS(t, pfx, nExts, exts_) \
    template<> struct ResLoadTraits<t> {                  \
        static std::string const prefix;                  \
        static std::array<char*, nExts> const exts;       \
    };                                                    \
    std::string const ResLoadTraits<t>::prefix(pfx);      \
    std::array<char*, nExts> const                        \
        ResLoadTraits<t>::exts = {LOAD_TRAITS_DETAIL_EXPAND_ARGS exts_};

std::string const findResource(
    std::string const& name,
    std::string const& prefix,
    char const* const* exts,
    std::size_t nExts);

template <typename ResT>
void loadSfmlResource(ResT& res, std::string const& name)
{
    std::string const filename = findResource(
        name,
        ResLoadTraits<ResT>::prefix,
        &ResLoadTraits<ResT>::exts.front(),
        ResLoadTraits<ResT>::exts.size());
    VFile f(filename);
    if (!res.loadFromStream(f)) {
        throw jd::ResourceLoadError(
            "failed loading resource \"" + name +
            "\" from file \"" + filename + "\"");
    }
}

void initDefaultResourceLoaders();

#endif
