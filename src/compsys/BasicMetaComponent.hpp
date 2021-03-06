// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef BASIC_META_COMPONENT_HPP_INCLUDED
#define BASIC_META_COMPONENT_HPP_INCLUDED BASIC_META_COMPONENT_HPP_INCLUDED

#include "ComponentRegistry.hpp"
#include "compsys/MetaComponent.hpp"

#include <luabind/back_reference.hpp> // for WeakRef (avoid linking errors)
#include <luabind/object.hpp>

#include <string>


template <typename T>
class BasicMetaComponent: public MetaComponent {
public:
    BasicMetaComponent() { ComponentRegistry::get().registerComponent(this); }
    virtual std::string const& name() const { return T::componentName; }
    virtual void castDown(lua_State* L, Component* c) const
    {
        assert(c->as<T>());
        luabind::object o(L, c->ref<T>());
        o.push(L);
    }
};

#define JD_BASIC_COMPONENT_IMPL(c) JD_COMPONENT_IMPL(c, BasicMetaComponent<c>)

#endif
