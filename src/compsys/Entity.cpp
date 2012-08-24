#include "Entity.hpp"
#include "EntityCollection.hpp"
#include "compsys/Component.hpp"
#include "Logfile.hpp"
#include <boost/foreach.hpp>

Entity::Entity(EntityCollection& parent, std::string const& id):
   m_parent(nullptr),
   m_id(id),
   m_state(created)
{
    parent.add(*this);
    assert(m_parent == &parent);
}

Entity::Entity(std::string const& id):
    m_parent(nullptr),
    m_id(id),
    m_state(created)
{
}

Entity::~Entity()
{
}

void Entity::add(Component& c)
{
    if (c.m_parent)
        throw std::invalid_argument("cannot add component: it already has a parent");
    if (m_state != created)
        throw std::logic_error("attempt to add a component to an Entity in a wrong state");
    if ((*this)[c.metaComponent()])
        throw std::logic_error("only one component per type per Entity allowed");
    c.m_parent = this;

    m_components.push_back(&c);
}

void Entity::finish()
{
    if (m_state == finished)
        return;
    if (m_state != created)
        throw std::logic_error("attempt to finish an Entity in a wrong state");
    m_state = finished;
    BOOST_FOREACH(Component& c, m_components)
        c.initComponent();
}
    
Component* Entity::operator[](MetaComponent const& meta)
{
    auto const it = std::find_if(m_components.begin(), m_components.end(),
    [&meta](Component const& c) {
        return &c.metaComponent() == &meta;
    });
    return it == m_components.end() ? nullptr : &*it;
}

void Entity::kill()
{
    if (m_state == killed)
        return;
    if (m_state != finished)
        throw std::logic_error("attempt to kill an Entity in a wrong state");
    m_state = killed;
    BOOST_FOREACH(Component& c, m_components)
        c.cleanupComponent();
}
