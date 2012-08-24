#include <SFML/System.hpp>

static char const libname[] = "SfSystem";
#include "ExportThis.hpp"
#include "luaexport/Geometry.hpp"
#include <luabind/operator.hpp>

static const lua_Number microsecondsPerSec = 1000000.0;

static lua_Number Time_asSeconds(sf::Time const& t)
{
    return t.asMicroseconds() / microsecondsPerSec;
}

static sf::Time Time_fromSeconds(lua_Number seconds)
{
    return sf::microseconds(
        static_cast<sf::Int64>(microsecondsPerSec * seconds));
}

static std::ostream& operator<< (std::ostream& os, sf::Time const& v)
{
    if (v.asSeconds() >= 1.f)
        return os << "jd.seconds(" << Time_asSeconds(v) << ')';
    return os << "jd.microseconds(" << v.asMicroseconds() << ')';
}


namespace sf {
static Time operator* (sf::Time lhs, lua_Number rhs)
{
    return sf::microseconds(
        static_cast<sf::Int64>(lhs.asMicroseconds() * rhs));
}
static Time operator/ (sf::Time lhs, lua_Number rhs)
{
    return sf::microseconds(
        static_cast<sf::Int64>(lhs.asMicroseconds() / rhs));
}

} // namespace sf

static void init(LuaVm& vm)
{
    vm.initLib(luaSfGeo::libname);
    using namespace sf;
    LHMODULE [
#       define LHCURCLASS Time
        LHCLASS
            .def("asSeconds", &Time_asSeconds)
            .LHMEMFN(asMilliseconds)
            .LHMEMFN(asMicroseconds)
            .def(const_self + sf::Time::Zero)
            .def(const_self - sf::Time::Zero)
            .def(const_self < sf::Time::Zero)
            .def(const_self <= sf::Time::Zero)
            .def(const_self == sf::Time::Zero)
            .def(const_self * lua_Number())
            .def(const_self / lua_Number())
            .def(tostring(const_self)),
#       undef LHCURCLASS
        def("seconds", &Time_fromSeconds),
        LHFN(milliseconds),
        LHFN(microseconds),
#       define LHCURCLASS Clock
        LHCLASS
            .def(constructor<>())
            .LHMEMFN(restart)
            .property("elapsedTime", &LHCURCLASS::getElapsedTime)
#       undef LHCURCLASS
    ];
}
