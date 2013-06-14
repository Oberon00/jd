#ifndef META_COMPONENT_HPP_INCLUDED
#define META_COMPONENT_HPP_INCLUDED META_COMPONENT_HPP_INCLUDED

#include <boost/noncopyable.hpp>

#include <cassert>
#include <stdexcept>
#include <string>


struct lua_State;
class ConnectionBase;
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
        assert(!"not scriptable!");
        throw std::runtime_error("castDown() is not implemented.");
    }

    // receiver is on top of stack; return value disconnects automatically on deletion
    virtual ConnectionBase* connectEvent(lua_State*, Component*, std::string const& name) const
    {
        (void)name;
        return nullptr;
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
    LuaMetaComponent(lua_State* L, std::string const& name);
    virtual std::string const& name() const;

    virtual void castDown(lua_State* L, Component* c) const override;
    virtual ConnectionBase* connectEvent(Component* c, std::string const& name) const;

private:
    lua_State* m_L;
    std::string const m_name;
};

#define JD_COMPONENT_IMPL(c, mc) \
    std::string const c::componentName(#c); \
    MetaComponent const& c::staticMetaComponent = mc();

#endif
