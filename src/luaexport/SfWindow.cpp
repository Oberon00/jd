#include <SFML/Window.hpp>
#include "sfUtil.hpp"

#include "LuaUtils.hpp"
#include "Geometry.hpp"

char const libname[] = "SfWindow";
#include "ExportThis.hpp"

#include <luabind/iterator_policy.hpp>
#include <luabind/operator.hpp>

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

#       define LHCURCLASS Event::KeyEvent
        class_<LHCURCLASS>("KeyEvent")
            .LHPROPRW(code)
            .LHPROPRW(alt)
            .LHPROPRW(control)
            .LHPROPRW(shift)
            .LHPROPRW(system),
#       undef LHCURCLASS

#       define LHCURCLASS Event::TextEvent
        class_<LHCURCLASS>("TextEvent")
            .LHPROPRW(unicode),
#       undef LHCURCLASS

#       define LHCURCLASS Event::SizeEvent
        class_<LHCURCLASS>("SizeEvent")
            .LHPROPRW(width)
            .LHPROPRW(height)
            .property("size", &SizeEvent_size),
#       undef LHCURCLASS

#       define LHCURCLASS Event::MouseMoveEvent
        class_<LHCURCLASS>("MouseMoveEvent")
            .LHPROPRW(x)
            .LHPROPRW(y)
            .property("position", &MouseEvent_position<LHCURCLASS>),
#       undef LHCURCLASS

#       define LHCURCLASS Event::MouseButtonEvent
        class_<LHCURCLASS>("MouseButtonEvent")
            .LHPROPRW(x)
            .LHPROPRW(y)
            .property("button", &MouseButtonEvent_button)
            .property("position", &MouseEvent_position<LHCURCLASS>),
#       undef LHCURCLASS


#       define LHCURCLASS Event::MouseWheelEvent
        class_<LHCURCLASS>("MouseWheelEvent")
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
