#ifndef ENTITY_HPP_INCLUDED
#define ENTITY_HPP_INCLUDED ENTITY_HPP_INCLUDED

#include <boost/ptr_container/ptr_vector.hpp>
#include <boost/noncopyable.hpp>
#include "WeakRef.hpp"
#include <string>

class Component;
class MetaComponent;

class EntityCollection;

class Entity: public EnableWeakRefFromThis<Entity>, private boost::noncopyable {
public:
    enum EntityState { created, finished, killed };

    Entity(EntityCollection& parent, std::string const& id = std::string());
    explicit Entity(std::string const& id = std::string());
    ~Entity();

    EntityCollection* parent() const { return m_parent; }
    void add(Component& c);
    std::string const& id() const { return m_id; }
    void setId(std::string const& id) { m_id = id; }
    void finish();
    void kill();
    EntityState state() const { return m_state; }

    Component* operator[](MetaComponent const& meta);
    
    template <typename T>
    T* get()
    {
        Component* c = (*this)[T::staticMetaComponent];
        return c ? c->as<T>() : nullptr;
    }

    template <typename T>
    T& require()
    {
        T* c = get<T>();
        if (!c) {
            std::string const typeName = typeid(T).name();
            throw std::runtime_error(
                "required component not available (type " + typeName + ')'); 
        }
        return *c;
    }

    bool operator== (Entity const& rhs) const { return this == &rhs; }
    bool operator!= (Entity const& rhs) const { return this != &rhs; }


private:
    Entity(Entity const&); // Get better error reports on MSVC

    friend EntityCollection;

    EntityCollection* m_parent;
    std::string m_id;
    EntityState m_state;
    boost::ptr_vector<Component> m_components;
};

#endif
