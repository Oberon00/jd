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

#define JD_EVT_METACOMPONENT(c, bc) \
    namespace {                                            \
    class c##Meta: public bc {                             \
    public:                                                \
        virtual ConnectionBase* connectEvent(lua_State*,   \
            Component*,                                    \
            std::string const& name) const;                \
    };                                                     \
    } // anonymous namespace


#define JD_BASIC_COMPONENT_IMPL(c) JD_COMPONENT_IMPL(c, BasicMetaComponent<c>)

#define JD_BASIC_EVT_COMPONENT_IMPL(c) \
    JD_EVT_METACOMPONENT(c, BasicMetaComponent<c>) \
    JD_COMPONENT_IMPL(c, c##Meta)



#endif
