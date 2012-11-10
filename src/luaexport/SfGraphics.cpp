#include <SFML/Graphics.hpp>
#include "ResourceManager.hpp"
#include "Geometry.hpp"
#include "AutoTexture.hpp"
#include "AutoFont.hpp"
#include "TransformGroup.hpp"
#include "sfUtil.hpp"
#include "VFileFont.hpp"

static char const libname[] = "SfGraphics";
#include "ExportThis.hpp"
#include "sharedPtrConverter.hpp"
#include <luabind/operator.hpp>
#include <ostream>

// sf::String <-> Lua converter (see luabind/detail/policy.hpp:741)
namespace luabind {
template <>
    struct default_converter<sf::String>
      : native_converter_base<sf::String>
    {
        static int compute_score(lua_State* L, int index)
        {
            return lua_type(L, index) == LUA_TSTRING ? 0 : -1;
        }

        sf::String from(lua_State* L, int index)
        {
            return sf::String(lua_tostring(L, index));
        }

        void to(lua_State* L, sf::String const& value)
        {
            std::string const s(value);
            lua_pushlstring(L, s.data(), s.size());
        }
    };

    template <>
    struct default_converter<sf::String const>
      : default_converter<sf::String>
    {};

    template <>
    struct default_converter<sf::String const&>
      : default_converter<sf::String>
    {};
} // namepace luabind

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

#define TEXT_STYLE_PROP(name) \
    static bool Text_is##name(sf::Text const& text)     \
    {                                                   \
        return (text.getStyle() & sf::Text::name) != 0; \
    }                                                   \
    static void Text_set##name(sf::Text& text, bool on) \
    {                                                   \
            text.setStyle(text.getStyle() & (on ?       \
                sf::Text::name : ~sf::Text::name));     \
    }

TEXT_STYLE_PROP(Bold)
TEXT_STYLE_PROP(Italic)
TEXT_STYLE_PROP(Underlined)
#undef TEXT_STYLE_PROP

static bool Text_isRegular(sf::Text const& text)
{
    return text.getStyle() == sf::Text::Regular;
}

static void Text_resetStyle(sf::Text& text)
{
    text.setStyle(sf::Text::Regular);
}

typedef AutoResource<sf::Sprite, sf::Texture> AutoSprite;
typedef GroupedDrawable<AutoSprite> SpriteEntry;

typedef GroupedDrawable<TransformGroup> GroupEntry;

typedef AutoResource<sf::Text, VFileFont> AutoText;
typedef GroupedDrawable<AutoText> TextEntry;



static void init(LuaVm& vm)
{
    using namespace sf;
    using namespace luabind;

#   define LHCURCLASS Image
    class_<LHCURCLASS, std::shared_ptr<LHCURCLASS>> cImage("Image");
    cImage
        .scope [
            def("create", &Image_create),
            def("create", &Image_createBlack)
        ]
        .def(constructor<LHCURCLASS const&>())
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
    class_<LHCURCLASS, std::shared_ptr<LHCURCLASS>> cTexture("Texture");
    cTexture
        .scope [
            def("maxSize", &LHCURCLASS::getMaximumSize)
        ]
        .property("repeated", &LHCURCLASS::isRepeated, &LHCURCLASS::setRepeated)
        .property("smooth", &LHCURCLASS::isSmooth, &LHCURCLASS::setSmooth)
        .LHMEMFN(copyToImage)
        .property("size", &LHCURCLASS::getSize);
#   undef LHCURCLASS
    addResMngMethods(cTexture);

#   define LHCURCLASS VFileFont
    class_<LHCURCLASS, std::shared_ptr<LHCURCLASS>, sf::Font> cFont("Font");
 
#   undef LHCURCLASS
    addResMngMethods(cFont);


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
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(r)
            .LHPROPRW(g)
            .LHPROPRW(b)
            .LHPROPRW(a)
            .def(const_self == other<sf::Color>())
            .def(const_self + other<sf::Color>())
            .def(const_self * other<sf::Color>())
            .def(tostring(const_self)),
#       undef LHCURCLASS

#		define LHCURCLASS TransformGroup::AutoEntry
		class_<LHCURCLASS>("TransformGroupEntry")
			.property("visible", &LHCURCLASS::visible, &LHCURCLASS::setVisible)
			.LHMEMFN(release),
#		undef LHCURCLASS

        cImage,
        cTexture,
        class_<Font>("@Font@"),
        cFont,
        class_<TransformGroup, bases<Transformable, Drawable>>("@TranformGroup@"),
		class_<GroupEntry, bases<TransformGroup, TransformGroup::AutoEntry>>("TransformGroup")
            .def(constructor<TransformGroup&>()),

		class_<Sprite, bases<Drawable, Transformable>>("@Sprite@"),
#       define LHCURCLASS SpriteEntry
        class_<LHCURCLASS, bases<Sprite, TransformGroup::AutoEntry>>("Sprite")
            .def(constructor<>())
            .def(constructor<LHCURCLASS const&>())
            .def(constructor<TransformGroup&>())
            .property("group", &LHCURCLASS::group, &LHCURCLASS::setGroup)
            .property("texture",
                (LHCURCLASS::Ptr(LHCURCLASS::*)())&LHCURCLASS::resource,
                &LHCURCLASS::setResource)
            .property("textureRect",
                &LHCURCLASS::getTextureRect, &LHCURCLASS::setTextureRect)
            .property("color",
                &LHCURCLASS::getColor, &LHCURCLASS::setColor)
            .property("localBounds", &LHCURCLASS::getLocalBounds)
            .property("bounds", &LHCURCLASS::getGlobalBounds),
#       undef LHCURCLASS

		class_<Text, bases<Drawable, Transformable>>("@Text@"),
#       define LHCURCLASS TextEntry
        class_<LHCURCLASS, bases<Text, TransformGroup::AutoEntry>>("Text")
            .def(constructor<>())
            .def(constructor<LHCURCLASS const&>())
            .def(constructor<TransformGroup&>())
            .property("group", &LHCURCLASS::group, &LHCURCLASS::setGroup)
            .property("font",
                (LHCURCLASS::Ptr(LHCURCLASS::*)())&LHCURCLASS::resource,
                &LHCURCLASS::setResource)
            .property("string",
                &LHCURCLASS::getString, &LHCURCLASS::setString)
            .property("characterSize",
                &LHCURCLASS::getCharacterSize, &LHCURCLASS::setCharacterSize)
            .property("bold", &Text_isBold, &Text_setBold)
            .property("italic", &Text_isItalic, &Text_setItalic)
            .property("underlined", &Text_isUnderlined, &Text_setUnderlined)
            .def("isRegular", &Text_isRegular)
            .def("resetStyle", &Text_resetStyle)
            .LHMEMFN(findCharacterPos)
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
    lua_getglobal(L, "jd");
    lua_getfield(L, -1, "Color");
    object stdColors(from_stack(L, -1));
    lua_pop(L, 2);

    stdColors["BLACK"]   = Color::Black;
    stdColors["WHITE"]   = Color::White;
    stdColors["RED"]     = Color::Red;
    stdColors["GREEN"]   = Color::Green;
    stdColors["BLUE"]    = Color::Blue;
    stdColors["YELLOW"]  = Color::Yellow;
    stdColors["MAGENTA"] = Color::Magenta;
    stdColors["CYAN"]    = Color::Cyan;
    stdColors["TRANSPARENT"] = Color::Transparent;
}
