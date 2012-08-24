#ifndef BASIC_META_COMPONENT_HPP_INCLUDED
#define BASIC_META_COMPONENT_HPP_INCLUDED BASIC_META_COMPONENT_HPP_INCLUDED

#include <string>
#include "compsys/MetaComponent.hpp"
#include "ComponentRegistry.hpp"
#include <luabind/object.hpp>
#include <luabind/back_reference.hpp> // for WeakRef (linking errors)

template <typename T>
class BasicSingletonMetaComponent: public MetaComponent {
public:
    BasicSingletonMetaComponent() { ComponentRegistry::get().registerComponent(this); }
    virtual std::string const& name() const { return T::componentName; }
    virtual T* create() const
    {
        throw std::logic_error("attempt to create a singleton [type=" +
            std::string(typeid(T).name()) + ']');
    }

    virtual void castUp(lua_State* L, Component* c) const
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


#define JD_SINGLETON_COMPONENT_IMPL(c) JD_COMPONENT_IMPL(c, BasicSingletonMetaComponent<c>)

#define JD_SINGLETON_EVT_COMPONENT_IMPL(c) \
    JD_EVT_METACOMPONENT(c, BasicSingletonMetaComponent<c>) \
    JD_COMPONENT_IMPL(c, c##Meta)

template <typename T>
class BasicMetaComponent: public BasicSingletonMetaComponent<T> {
public:
    virtual T* create() const { return new T; }
};

#define JD_BASIC_COMPONENT_IMPL(c) JD_COMPONENT_IMPL(c, BasicMetaComponent<c>)

#define JD_BASIC_EVT_COMPONENT_IMPL(c) \
    JD_EVT_METACOMPONENT(c, BasicMetaComponent<c>) \
    JD_COMPONENT_IMPL(c, c##Meta)



#endif