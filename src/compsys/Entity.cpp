// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "Entity.hpp"

#include "Logfile.hpp"


Entity::Entity():
    m_state(EntityState::created)
{
}

Entity::~Entity()
{
    if (m_state == EntityState::finished) {
        try { kill(); }
        catch (std::exception const& e) {
            LOG_E("Exception in Entity destructor: kill() threw:");
            LOG_EX(e);
        } catch (...) {
            LOG_F("Exception in Entity destructor:"
                  " kill() threw an unknown exception. Terminating.");
            std::terminate();
        }
    }
}

void Entity::add(Component& c)
{
    if (c.m_parent)
        throw std::logic_error("cannot add component: it already has a parent");
    if (m_state != EntityState::created)
        throw std::logic_error("attempt to add a component to an Entity in a wrong state");
    if ((*this)[c.metaComponent()])
        throw std::logic_error("only one component per type per Entity allowed");
    c.m_parent = this;

    m_components.push_back(&c);
}

void Entity::finish()
{
    if (m_state == EntityState::finished)
        return;
    if (m_state != EntityState::created)
        throw std::logic_error("attempt to finish an Entity in a wrong state");
    m_state = EntityState::finished;
    for (Component& c : m_components)
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
    if (m_state == EntityState::killed)
        return;
    if (m_state != EntityState::finished)
        throw std::logic_error("attempt to kill an Entity in a wrong state");
    m_state = EntityState::killed;
    for (Component& c : m_components)
        c.cleanupComponent();
}
