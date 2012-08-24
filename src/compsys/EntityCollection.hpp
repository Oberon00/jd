#ifndef ENTITY_COLLECTION_HPP_INCLUDED
#define ENTITY_COLLECTION_HPP_INCLUDED ENTITY_COLLECTION_HPP_INCLUDED

#include <string>
#include <boost/ptr_container/ptr_list.hpp>
#include <boost/noncopyable.hpp>
#include "WeakRef.hpp"
#include "Entity.hpp" // boost.PtrContainer demands this for destruction

class EntityCollection:
    public EnableWeakRefFromThis<EntityCollection>,
    private boost::noncopyable {
public:
    explicit EntityCollection(EntityCollection& parent, std::string const& id = std::string()):
        m_id(id), m_killed(false), m_parent(nullptr)
    {
        parent.add(*this);
    }

    explicit EntityCollection(std::string const& id = std::string()):
        m_id(id), m_killed(false), m_parent(nullptr) { }

    ~EntityCollection();

    void add(Entity& e);
    void add(EntityCollection& ec);


    Entity* operator[] (std::string const& id);
    EntityCollection* collection(std::string const& id);

    EntityCollection* parent() { return m_parent; }

    void clear();
    void tidy(); // remove killed entities

    std::string const& id() const { return m_id; }
    void setId(std::string const& id) { m_id = id; }

    void kill() { m_killed = true; }
    bool killed() const { return m_killed; }

    bool operator== (EntityCollection const& rhs) const { return this == &rhs; }
    bool operator!= (EntityCollection const& rhs) const { return this != &rhs; }


private:
    bool m_killed;
    std::string m_id;
    EntityCollection* m_parent;
    boost::ptr_list<Entity> m_entities;
    boost::ptr_list<EntityCollection> m_collections;
};
#endif