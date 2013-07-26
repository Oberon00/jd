// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef META_COMPONENT_HPP_INCLUDED
#define META_COMPONENT_HPP_INCLUDED META_COMPONENT_HPP_INCLUDED

#include <boost/noncopyable.hpp>

#include <cassert>
#include <stdexcept>
#include <string>


struct lua_State;
class Component;

class InvalidMetaComponentName: public std::runtime_error {
public:
    InvalidMetaComponentName(std::string const& name):
        std::runtime_error("MetaComponent \"" + name + "\" is not registered.") { }
};

/* abstract */ class MetaComponent: private boost::noncopyable  {
public:
    virtual ~MetaComponent() { }

    virtual std::string const& name() const = 0;

    // places result on top of stack.
    virtual void castDown(lua_State*, Component*) const
    {
        assert("not scriptable!" && false);
        throw std::runtime_error("castDown() is not implemented.");
    }
};

// Two MetaComponent instances are equal if and only if they have the same
// adresses. In addition, only equal MetaComponents may have equal names.
bool operator== (MetaComponent const& lhs, MetaComponent const& rhs);
bool operator!= (MetaComponent const& lhs, MetaComponent const& rhs);

// Sorting, however is not by adress but by name.
bool operator<  (MetaComponent const& lhs, MetaComponent const& rhs);


class LuaMetaComponent: public MetaComponent {
public:
    explicit LuaMetaComponent(lua_State* L, std::string const& name);
    std::string const& name() const override;

    void castDown(lua_State* L, Component* c) const override;

private:
    std::string const m_name;
};

#define JD_COMPONENT_IMPL(c, mc) \
    std::string const c::componentName(#c); \
    MetaComponent const& c::staticMetaComponent = mc();

#endif
