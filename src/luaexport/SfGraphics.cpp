#include "AutoFont.hpp"
#include "AutoTexture.hpp"
#include "ResourceManager.hpp"
#include "resources.hpp"
#include "sharedPtrConverter.hpp"
#include "SfBaseTypes.hpp"
#include "sfUtil.hpp"
#include "TransformGroup.hpp"
#include "VFileFont.hpp"

#include <luabind/copy_policy.hpp>
#include <luabind/operator.hpp>
#include <SFML/Graphics.hpp>

#include <ostream>

static char const libname[] = "SfGraphics";
#include "ExportThis.hpp"

static_assert(
    luabind::detail::has_get_pointer<std::shared_ptr<sf::Image>>::value,
    "get_pointer for std::shared_ptr broke again");


template <typename T, typename B>
static void addDrawableDefs(luabind::class_<T, B >& c)
{
    c
        .def(luabind::constructor<>())
        .def(luabind::constructor<T const&>())
        .def(luabind::constructor<TransformGroup&>())
        .property("group", &T::group, &T::setGroup)
        .property("localBounds", &T::getLocalBounds)
        .property("bounds", &T::getGlobalBounds);
}

template <typename T, typename B>
static void addTextureProp(luabind::class_<T, B >& c)
{
    c.property("texture",
        (typename T::Ptr(T::*)())&T::resource,
        &T::setResource);
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
    static bool Text_is##name(sf::Text const& text)                \
    {                                                              \
        return (text.getStyle() & sf::Text::name) != 0;            \
    }                                                              \
    static void Text_set##name(sf::Text& text, bool on)            \
    {                                                              \
            text.setStyle(on ? text.getStyle() | sf::Text::name    \
                             : text.getStyle() & ~sf::Text::name); \
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

static sf::Vector2f Shape_point(sf::Shape const& s, unsigned idx)
{
    if (idx >= s.getPointCount())
        throw "point index of shape out of range";
    return s.getPoint(idx);
}

typedef AutoResource<sf::Sprite, sf::Texture> AutoSprite;
typedef GroupedDrawable<AutoSprite> SpriteEntry;

typedef GroupedDrawable<TransformGroup> GroupEntry;

typedef AutoResource<sf::Text, VFileFont> AutoText;
typedef GroupedDrawable<AutoText> TextEntry;

typedef AutoResource<sf::RectangleShape, sf::Texture> AutoRectangleShape;
typedef GroupedDrawable<AutoRectangleShape> RectangleShapeEntry;

typedef AutoResource<sf::CircleShape, sf::Texture> AutoCircleShape;
typedef GroupedDrawable<AutoCircleShape> CircleShapeEntry;


static void init(LuaVm& vm)
{
    vm.initLib("SfWindow"); // RenderWindow derives from Window
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
    cFont
        .def("lineSpacing", &LHCURCLASS::getLineSpacing);
#   undef LHCURCLASS
    addResMngMethods(cFont);

#   define LHCURCLASS SpriteEntry
    class_<LHCURCLASS, bases<Sprite, TransformGroup::AutoEntry>> cSprite("Sprite");
    cSprite
        .property("textureRect",
            &LHCURCLASS::getTextureRect, &LHCURCLASS::setTextureRect)
        .property("color",
            &LHCURCLASS::getColor, &LHCURCLASS::setColor, copy(result));
#   undef LHCURCLASS
    addDrawableDefs(cSprite);
    addTextureProp(cSprite);

#   define LHCURCLASS RectangleShapeEntry
        class_<LHCURCLASS, bases<RectangleShape, TransformGroup::AutoEntry>> cRectangleShape("RectangleShape");
        cRectangleShape
            .property("size", &LHCURCLASS::getSize, &LHCURCLASS::setSize);
#   undef LHCURCLASS
    addDrawableDefs(cRectangleShape);
    addTextureProp(cRectangleShape);

#   define LHCURCLASS CircleShapeEntry
        class_<LHCURCLASS, bases<CircleShape, TransformGroup::AutoEntry>> cCircleShape("CircleShape");
        cRectangleShape
            .property("radius", &LHCURCLASS::getRadius, &LHCURCLASS::setRadius)
            .property("pointCount", &LHCURCLASS::getPointCount, &LHCURCLASS::setPointCount);
#   undef LHCURCLASS
    addDrawableDefs(cRectangleShape);
    addTextureProp(cRectangleShape);

#   define LHCURCLASS TextEntry
        class_<LHCURCLASS, bases<Text, TransformGroup::AutoEntry>> cText("Text");
        cText
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
            .def("breakLines", &jd::breakTextLines)
            .LHMEMFN(findCharacterPos)
            .property("color",
                &LHCURCLASS::getColor, &LHCURCLASS::setColor, copy(result));
#   undef LHCURCLASS
    addDrawableDefs(cText);

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
#       define LHCURCLASS RenderWindow
        class_<LHCURCLASS, Window>("RenderWindow")
            .def(constructor<>())
            .def(constructor<VideoMode, sf::String const&>())
            .def(constructor<VideoMode, sf::String const&, Uint32>())
            .def(constructor<VideoMode, sf::String const&, Uint32,
                ContextSettings const&>()),
#       undef LHCURCLASS

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

#       define LHCURCLASS TransformGroup::AutoEntry
        class_<LHCURCLASS>("TransformGroupEntry")
            .property("visible", &LHCURCLASS::visible, &LHCURCLASS::setVisible)
            .LHMEMFN(release),
#       undef LHCURCLASS

        cImage,
        cTexture,
        class_<Font>("@Font@"),
        cFont,
        class_<TransformGroup, bases<Transformable, Drawable>>("@TranformGroup@"),

#       define LHCURCLASS GroupEntry
        class_<GroupEntry, bases<TransformGroup, TransformGroup::AutoEntry>>("TransformGroup")
            .def(constructor<>())
            .def(constructor<TransformGroup&>())
            .property("group", &LHCURCLASS::group, &LHCURCLASS::setGroup),
#       undef LHCURCLASS

        class_<Sprite, bases<Drawable, Transformable>>("@Sprite@"),
        cSprite,

        class_<Text, bases<Drawable, Transformable>>("@Text@"),
        cText,

#       define LHCURCLASS Shape
        class_<LHCURCLASS, bases<Drawable, Transformable>>("Shape")
            .property("localBounds", &LHCURCLASS::getLocalBounds)
            .property("bounds", &LHCURCLASS::getGlobalBounds)
            .property("fillColor",
                &LHCURCLASS::getFillColor, &LHCURCLASS::setFillColor, copy(result))
            .property("outlineColor",
                &LHCURCLASS::getOutlineColor, &LHCURCLASS::setOutlineColor, copy(result))
            .property("outlineThickness",
                &LHCURCLASS::getOutlineThickness, &LHCURCLASS::setOutlineThickness)
            .property("pointCount", &LHCURCLASS::getPointCount)
            .def("point", &Shape_point)
            .property("textureRect",
                &LHCURCLASS::getTextureRect, &LHCURCLASS::setTextureRect),
#       undef LHCURCLASS

        class_<RectangleShape, Shape>("@RectangleShape@"),
        cRectangleShape,

        class_<CircleShape, Shape>("@CircleShape@"),
        cCircleShape,

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
