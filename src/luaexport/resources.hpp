#ifndef RESOURCES_HPP_INCLUDED
#define RESOURCES_HPP_INCLUDED RESOURCES_HPP_INCLUDED

#include "ressys/ResourceManager.hpp"

#include <luabind/luabind.hpp>


#define RESMNG_METHOD(r, n, a, a2) \
    template <typename T>          \
    static r ResMng_##n a { return resMng<T>().n a2; }

#define M(n) RESMNG_METHOD( \
    typename ResourceManager<T>::Ptr, n, \
    (std::string const& name), (name))
M(get)
M(request)
M(tryGet)
M(keepLoaded)
#undef M

#define M(n) RESMNG_METHOD(void, n, (), ())
M(releaseAll)
M(tidy)
M(purge)
#undef M

RESMNG_METHOD(void, release, (std::string const& name), (name))
RESMNG_METHOD(void, insert, \
    (std::string const& name, typename ResourceManager<T>::Ptr p), (name, p))

#undef RESMNG_METHOD

template <typename T, typename B>
static void addResMngMethods(luabind::class_<T, std::shared_ptr<T>, B >& c)
{
#define F(n) luabind::def(#n, &ResMng_##n<T>)
    c.scope [
        F(get),
        F(tryGet),
        F(request),
        F(keepLoaded),
        F(releaseAll),
        F(tidy),
        F(purge),
        F(release),
        F(insert)
    ];
#undef F
}

#endif //RESOURCES_HPP_INCLUDED
