// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef COMPONENT_REGISTRY_HPP_INCLUDED
#define COMPONENT_REGISTRY_HPP_INCLUDED COMPONENT_REGISTRY_HPP_INCLUDED

#include <string>
#include <unordered_map>


struct lua_State;
class MetaComponent;

class ComponentRegistry {
public:
    void registerComponent(MetaComponent const* meta);
    MetaComponent const* operator[] (std::string const& name);

    static ComponentRegistry& get(); // get the C++ ComponentRegistry
    static ComponentRegistry& get(lua_State* L); // get the Lua ComponentRegistry for L
    static MetaComponent const& metaComponent(std::string const& name, lua_State* L = nullptr);

    static void _init(lua_State* L);

private:
    ComponentRegistry() { }
    std::unordered_map<std::string, MetaComponent const*> m_registry;
};

#endif
