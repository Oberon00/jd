#include <SFML/Graphics.hpp>
#include "ResourceManager.hpp"
#include "Geometry.hpp"
#include "AutoTexture.hpp"
#include "TransformGroup.hpp"
#include "sfUtil.hpp"

static char const libname[] = "SfGraphics";
#include "ExportThis.hpp"
#include "sharedPtrConverter.hpp"
#include <luabind/operator.hpp>
#include <ostream>

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

template <typename T>
static void addResMngMethods(luabind::class_<T, std::shared_ptr<T> >& c)
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

std::ostream& operator<< (std::ostream& o, sf::Color c)
{
    return o << "jd.Color("
             << static_cast<unsigned>(c.r) << ','
             << static_cast<unsigned>(c.g) << ','
             << static_cast<unsigned>(c.b) << ','
             << static_cast<unsigned>(c.a) << ')';
}

static std::shared_ptr<sf::Image> Image_create(
    unsigned w, unsigned h, sf::Color c)
{
    auto img = std::make_shared<sf::Image>();
    img->create(w, h, c);
    return img;
}

static std::shared_ptr<sf::Image> Image_createBlack(
    unsigned w, unsigned h)
{
    return Image_create(w, h, sf::Color::Black);
}

static sf::Color Image_pixel(sf::Image const& img, unsigned x, unsigned y)
{
    if (x >= img.getSize().x || y >= img.getSize().y)
        throw "pixel coordinates of image out of range (@get)";
    return img.getPixel(x, y);
}

static void Image_setPixel(
    sf::Image& img, unsigned x, unsigned y, sf::Color c)
{
    if (x >= img.getSize().x || y >= img.getSize().y)
        throw "pixel coordinates of image out of range (@set)";
    img.setPixel(x, y, c);
}

static void Image_transparentMask(sf::Image& img, sf::Color const& c)
{
    img.createMaskFromColor(c);
}

typedef AutoResource<sf::Sprite, sf::Texture> AutoSprite;
typedef GroupedDrawable<AutoSprite> SpriteEntry;

typedef GroupedDrawable<TransformGroup> GroupEntry;


static void init(LuaVm& vm)
{
    using namespace sf;
    using namespace luabind;

#   define LHCURCLASS Image
    class_<Image, std::shared_ptr<Image>> cImage("Image");
    cImage
        .scope [
            def("create", &Image_create),
            def("create", &Image_createBlack)
        ]
        .property("size", &LHCURCLASS::getSize)
        .def("createTransparentMask", &LHCURCLASS::createMaskFromColor)
        .def("createTransparentMask", &Image_transparentMask)
        .def("pixel", &Image_pixel)
        .def("setPixel", &Image_setPixel)
        .LHMEMFN(flipHorizontally)
        .LHMEMFN(flipVertically);
#   undef LHCURCLASS
    addResMngMethods(cImage);

#   define LHCURCLASS Texture
    class_<Texture, std::shared_ptr<Texture>> cTexture("Texture");
    cTexture
        .scope [
            def("maxSize", &Texture::getMaximumSize)
        ]
        .property("repeated", &Texture::isRepeated, &Texture::setRepeated)
        .property("smooth", &Texture::isSmooth, &Texture::setSmooth)
        .LHMEMFN(copyToImage)
        .property("size", &Texture::getSize);
#   undef LHCURCLASS
    addResMngMethods(cTexture);

#define XYPROP(n, N) \
    .property(BOOST_STRINGIZE(n), \
        &LHCURCLASS::get##N,      \
        (void(LHCURCLASS::*)(Vector2f const&))&LHCURCLASS::set##N) \
    .def(BOOST_STRINGIZE(set##N), \
        (void(LHCURCLASS::*)(float, float))&LHCURCLASS::set##N)

// ln = Lua name; n = C++ name
#define XYMEMFN2(ln, n)  \
    .def(BOOST_STRINGIZE(ln), \
        (void(LHCURCLASS::*)(float, float))&LHCURCLASS::n)    \
    .def(BOOST_STRINGIZE(ln), \
        (void(LHCURCLASS::*)(Vector2f const&))&LHCURCLASS::n)

#define XYMEMFN(n) XYMEMFN2(n, n)

    LHMODULE [
        LHCLASS2(Drawable),
#       define LHCURCLASS Transformable
        LHCLASS
            XYPROP(position, Position)
            .property("rotation",
                &LHCURCLASS::getRotation,
                &LHCURCLASS::setRotation)
            XYPROP(scale, Scale)
            XYPROP(origin, Origin)
            XYMEMFN(move)
            XYMEMFN2(scaleBy, scale),
#       undef LHCURCLASS

#       define LHCURCLASS Color
        LHCLASS
            .def(constructor<>())
            .def(constructor<Uint8, Uint8, Uint8>())
            .def(constructor<Uint8, Uint8, Uint8, Uint8>())
            .LHPROPRW(r)
            .LHPROPRW(g)
            .LHPROPRW(b)
            .LHPROPRW(a)
            .def(const_self == other<sf::Color>())
            .def(const_self + other<sf::Color>())
            .def(const_self * other<sf::Color>())
            .def(tostring(const_self)),
#       undef LHCURCLASS

        cImage,
        cTexture,
        class_<TransformGroup, bases<Transformable, Drawable>>("@TranformGroup@"),
        class_<GroupEntry, TransformGroup>("TransformGroup")
            .def(constructor<>())
            .def(constructor<TransformGroup&>()),

#       define LHCURCLASS SpriteEntry
        class_<LHCURCLASS, bases<Transformable, Drawable>>("Sprite")
            .def(constructor<>())
            .def(constructor<TransformGroup&>())
            .LHMEMFN(release)
            .property("visible", &LHCURCLASS::visible, &LHCURCLASS::setVisible)
            .property("texture",
                (LHCURCLASS::Ptr(LHCURCLASS::*)())&LHCURCLASS::resource,
                &AutoSprite::setResource)
            .property("textureRect",
                &LHCURCLASS::getTextureRect, &LHCURCLASS::setTextureRect)
            .property("color",
                &LHCURCLASS::getColor, &LHCURCLASS::setColor)
            .property("localBounds", &LHCURCLASS::getLocalBounds)
            .property("bounds", &LHCURCLASS::getGlobalBounds),
#       undef LHCURCLASS

#       define LHCURCLASS View
        LHCLASS
            .def(constructor<>())
            .def(constructor<FloatRect>())
            .def(constructor<Vector2f, Vector2f>())
            XYPROP(center, Center)
            XYPROP(size, Size)
            XYMEMFN(move)
            .property("viewport",
                &LHCURCLASS::getViewport,
                &LHCURCLASS::setViewport)
            .LHMEMFN(zoom)
            .LHMEMFN(rotate)
            .property("rect", &jd::viewRect, &jd::setViewRect)
#       undef LHCURCLASS

    ];

    lua_State* L = vm.L();
    static int const colorCount = 9;
    lua_createtable(L, 0, colorCount);
    object stdColors(from_stack(L, -1));
    lua_pop(L, 1);

    stdColors["BLACK"]   = Color::Black;
    stdColors["WHITE"]   = Color::White;
    stdColors["RED"]     = Color::Red;
    stdColors["GREEN"]   = Color::Green;
    stdColors["BLUE"]    = Color::Blue;
    stdColors["YELLOW"]  = Color::Yellow;
    stdColors["MAGENTA"] = Color::Magenta;
    stdColors["CYAN"]    = Color::Cyan;
    stdColors["TRANSPARENT"] = Color::Transparent;

    globals(L)[jd::moduleName]["colors"] = stdColors;
}
