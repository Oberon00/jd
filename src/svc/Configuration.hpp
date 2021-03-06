// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef CONFIGURATION_HPP_INCLUDED
#define CONFIGURATION_HPP_INCLUDED CONFIGURATION_HPP_INCLUDED

#include "luaUtils.hpp"
#include "LuaVm.hpp"

#include <luabind/object.hpp>

#include <string>


class Configuration {
public:
    Configuration(lua_State* L);
    
    class Error: public std::runtime_error {
    public:
        Error(std::string const& msg):
          std::runtime_error(msg) { }
        Error(std::string const& p, std::string const& msg):
          std::runtime_error("error retrieving configuration value at \"" + p + "\": " + msg) { }
    };

    void load(std::string const& configurationFilename = "configuration.lua",
        std::string const& globalName = "conf");

    void reload();
    void save();

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
    std::string m_confPath;
    luabind::object m_conf;
};
#endif
