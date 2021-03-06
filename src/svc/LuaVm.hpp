// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef LUA_VM_HPP_INCLUDED
#define LUA_VM_HPP_INCLUDED LUA_VM_HPP_INCLUDED

#include <boost/function.hpp>
#include <luabind/error.hpp>

#include <stdexcept>
#include <string>
#include <unordered_map>


class MetaComponent;
struct lua_State;

class LuaVm {
public:

    explicit LuaVm(std::string const& libConfigFilename = "luainit.lua");
    virtual ~LuaVm();

    typedef boost::function<void(LuaVm&)> LibInitFn;
    static void registerLib(std::string const& libname, LibInitFn const& initFn);
    void initLib(std::string const& libname);
    void initLibs();

    void deinit();

    lua_State* L() { return m_L; }

    static LuaVm& get(lua_State* L);

private:
    struct LibInfo
    {
        explicit LibInfo(LibInitFn const& initFn): initialized(false), initFn(initFn) { }
        bool initialized;
        LibInitFn initFn;
    };

    typedef std::unordered_map<std::string, LibInfo> LibRegistry;
    static LibRegistry& libRegistry()
    {
        static LibRegistry reg;
        return reg;
    }

    lua_State* m_L;
};

#endif
