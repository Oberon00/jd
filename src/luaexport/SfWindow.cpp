#include "SfBaseTypes.hpp"
#include "sfUtil.hpp"
#include "LuaUtils.hpp"

#include <luabind/iterator_policy.hpp>
#include <luabind/operator.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Image.hpp> // for Window_setIcon

char const libname[] = "SfWindow";
#include "ExportThis.hpp"


static sf::VideoMode getFullscreenMode(std::size_t i)
{
    return sf::VideoMode().getFullscreenModes().at(i - 1);
}

static std::size_t getFullscreenModeCount()
{
    return sf::VideoMode().getFullscreenModes().size();
}

static std::ostream& operator<< (std::ostream& os, sf::VideoMode const& v)
{
    return os << "jd.VideoMode(" << v.width << ',' << v.height << ',' << v.bitsPerPixel << ')';
}

template <typename MouseEvent>
static LuaVec2 MouseEvent_position(MouseEvent const& ev)
{
    return LuaVec2(
        static_cast<lua_Number>(ev.x),
        static_cast<lua_Number>(ev.y));
}

static LuaVec2 SizeEvent_size(sf::Event::SizeEvent const& ev)
{
    return LuaVec2(
        static_cast<lua_Number>(ev.width),
        static_cast<lua_Number>(ev.height));
}

static unsigned MouseButtonEvent_button(sf::Event::MouseButtonEvent const& ev)
{
    return static_cast<unsigned>(ev.button);
}

static LuaVec2 VideoMode_size(sf::VideoMode const& mode)
{
    return LuaVec2(
        static_cast<lua_Number>(mode.width),
        static_cast<lua_Number>(mode.height));
}

static void Window_create2(sf::Window& w,
    sf::VideoMode mode, sf::String const& title)
{
    w.create(mode, title);
}

static void Window_create3(sf::Window& w,
     sf::VideoMode mode, sf::String const& title, sf::Uint32 style)
{
    w.create(mode, title, style);
}

static void Window_setIcon(sf::Window& w, sf::Image const& icon)
{
    if (!icon.getPixelsPtr())
        throw std::invalid_argument("Cannot set an empty window-icon.");
    w.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
}



static void init(LuaVm& vm)
{
    using namespace sf;
    using jd::keyName;

    LHMODULE [
#       define LHCURCLASS VideoMode
        LHCLASS
            .def(constructor<>())
            .def(constructor<unsigned, unsigned>())
            .def(constructor<unsigned, unsigned, unsigned>())
            .def(constructor<LHCURCLASS const&>())
            .property("size", &VideoMode_size)
            .LHPROPG(isValid)
            .LHPROPRW(width)
            .LHPROPRW(height)
            .LHPROPRW(bitsPerPixel)
            .def(const_self <  other<VideoMode>())
            .def(const_self <= other<VideoMode>())
            .def(const_self == other<VideoMode>())
            .def(tostring(const_self))
            .scope [
                def("desktopMode", &VideoMode::getDesktopMode),
                def("all", &VideoMode::getFullscreenModes,
                    return_stl_iterator),
                def("get", &getFullscreenMode),
                def("count", &getFullscreenModeCount)
            ],
#       undef LHCURCLASS

#       define LHCURCLASS ContextSettings
        LHCLASS
            .def(constructor<>())
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(depthBits)
            .LHPROPRW(stencilBits)
            .LHPROPRW(antialiasingLevel)
            .LHPROPRW(majorVersion)
            .LHPROPRW(minorVersion),
#       undef LHCURCLASS


#       define LHCURCLASS Window
        LHCLASS
            .def("create",
                (void (LHCURCLASS::*)(VideoMode, sf::String const&, Uint32,
                      ContextSettings const&))&LHCURCLASS::create)
            .def("create", &Window_create2)
            .def("create", &Window_create3)
            .property("position",
                &LHCURCLASS::getPosition, &LHCURCLASS::setPosition)
            .property("size", &LHCURCLASS::getSize, &LHCURCLASS::setSize)
            .def("setIcon", &Window_setIcon)
            .def("setTitle", &LHCURCLASS::setTitle)
            .def("setMouseCursorVisible", &LHCURCLASS::setMouseCursorVisible)
            .def("setFramerateLimit", &LHCURCLASS::setFramerateLimit)
            .property("contextSettings", &LHCURCLASS::getSettings)
            .LHMEMFN(close)
            .property("isOpen", &LHCURCLASS::isOpen)
            .def("setVSyncEnabled", &LHCURCLASS::setVerticalSyncEnabled)
            .def("setKeyRepeatEnabled", &LHCURCLASS::setKeyRepeatEnabled)
            .enum_("Style") [
                value("STYLE_NONE", sf::Style::None),
                value("STYLE_TITLEBAR", sf::Style::Titlebar),
                value("STYLE_RESIZE", sf::Style::Resize),
                value("STYLE_CLOSE", sf::Style::Close),
                value("STYLE_FULLSCREEN", sf::Style::Fullscreen),
                value("STYLE_DEFAULT", sf::Style::Default)
            ],
#       undef LHCURCLASS



#       define LHCURCLASS Event::KeyEvent
        class_<LHCURCLASS>("KeyEvent")
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(code)
            .LHPROPRW(alt)
            .LHPROPRW(control)
            .LHPROPRW(shift)
            .LHPROPRW(system),
#       undef LHCURCLASS

#       define LHCURCLASS Event::TextEvent
        class_<LHCURCLASS>("TextEvent")
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(unicode),
#       undef LHCURCLASS

#       define LHCURCLASS Event::SizeEvent
        class_<LHCURCLASS>("SizeEvent")
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(width)
            .LHPROPRW(height)
            .property("size", &SizeEvent_size),
#       undef LHCURCLASS

#       define LHCURCLASS Event::MouseMoveEvent
        class_<LHCURCLASS>("MouseMoveEvent")
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(x)
            .LHPROPRW(y)
            .property("position", &MouseEvent_position<LHCURCLASS>),
#       undef LHCURCLASS

#       define LHCURCLASS Event::MouseButtonEvent
        class_<LHCURCLASS>("MouseButtonEvent")
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(x)
            .LHPROPRW(y)
            .property("button", &MouseButtonEvent_button)
            .property("position", &MouseEvent_position<LHCURCLASS>),
#       undef LHCURCLASS


#       define LHCURCLASS Event::MouseWheelEvent
        class_<LHCURCLASS>("MouseWheelEvent")
            .def(constructor<LHCURCLASS const&>())
            .LHPROPRW(x)
            .LHPROPRW(y)
            .LHPROPRW(delta)
            .property("position", &MouseEvent_position<LHCURCLASS>),
#       undef LHCURCLASS

#       define LHCURCLASS Keyboard
        namespace_("kb") [
            LHFN(keyName)
        ]
#       undef LHCURCLASS
    ];

    lua_State* L = vm.L();
    lua_getglobal(L, "jd");
    lua_getfield(L, -1, "kb");
    static luaU::ExportedEnumValue const keys[] = {
#       define E(n, v) {n, Keyboard::v},
#       include "sfKeyCodes.hpp"
        {nullptr, 0}
    };
    luaU::exportenum(L, -1, keys);
    lua_pop(L, 2);
}
