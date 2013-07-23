// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef COMPONENT_HPP_INCLUDED
#define COMPONENT_HPP_INCLUDED COMPONENT_HPP_INCLUDED

#include "WeakRef.hpp"

#include <boost/noncopyable.hpp>

#include <cassert>
#include <iosfwd>


class MetaComponent;
class Entity;

/* abstract */ class Component: public EnableWeakRefFromThis<Component>, private boost::noncopyable {
public:
    Component(): m_parent(nullptr) { }
    virtual ~Component() = 0;
    virtual MetaComponent const& metaComponent() const = 0;
    virtual void initComponent() { }
    virtual void cleanupComponent() { }

    Entity* parent() const { return m_parent; }

    template<typename T>
    T* as() // unsafe & fast
    {
        assert(&T::staticMetaComponent == &metaComponent());
        assert(dynamic_cast<T*>(this));
        return static_cast<T*>(this);
    }

    bool operator== (Component const& rhs) const { return this == &rhs; }
    bool operator!= (Component const& rhs) const { return this != &rhs; }

private:
    friend Entity;
    Entity* m_parent;
};

template<typename T>
inline T* component_cast(Component* c) // safe & a bit slower
{
    if (!c || &T::staticMetaComponent != &c->metaComponent())
        return nullptr;
    assert(dynamic_cast<T*>(c));
    return static_cast<T*>(c);
}

#define JD_COMPONENT \
    public:                                              \
        static std::string const componentName;          \
        static class MetaComponent const& staticMetaComponent; \
        virtual class MetaComponent const& metaComponent() const { return staticMetaComponent; } \
    private:

        
std::ostream& operator<< (std::ostream& os, Component const& c);

#endif
