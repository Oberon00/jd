#ifndef CONFIGURATION_HPP_INCLUDED
#define CONFIGURATION_HPP_INCLUDED CONFIGURATION_HPP_INCLUDED

#include "compsys/Component.hpp"
#include "jdConfig.hpp"
#include "LuaVm.hpp"
#include <string>
#include <luabind/object.hpp>

class Configuration: public Component {
    JD_COMPONENT
public:

    class Error: public std::runtime_error {
    public:
        Error(std::string const& msg):
          std::runtime_error(msg) { }
        Error(std::string const& p, std::string const& msg):
          std::runtime_error("error retrieving configuration value at \"" + p + "\": " + msg) { }
    };

    void load(std::string const& configurationFilename = "configuration.lua",
        std::string const& globalName = "conf");

    template <typename T>
    T get(std::string const& p, T const& def)
    {
        try {
            return get<T>(p);
        } catch (Error const&) {
            return def;
        }
    }

    template <typename T>
    T get(std::string const& p)
    {
        try {
            return luabind::object_cast<T>(getObject(p));
        } catch (luabind::cast_failed const& e) {
            throw Error(p, e.what());
        } catch (luabind::error const& e) {
            throw Error(p, luaU::Error(e).what());
        }
    }


    luabind::object getObject(std::string const& p);

private:
    luabind::object m_conf;
};
#endif