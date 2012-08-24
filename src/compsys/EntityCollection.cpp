#include "EntityCollection.hpp"
#include "compsys/Component.hpp"
#include <boost/foreach.hpp>
#include <algorithm>

template<typename T>
static T getsub(EntityCollection& ec, std::string const& id);

template<>
static EntityCollection* getsub<EntityCollection*>(EntityCollection& ec, std::string const& id)
{
    return ec.collection(id);
}

template<>
static Entity* getsub<Entity*>(EntityCollection& ec, std::string const& id)
{
    return ec[id];
}

template <typename T>
static typename T::pointer findEntityOrCollection(
    T& container,
    boost::ptr_list<EntityCollection>& sub,
    std::string const& id)
{
    if (id.empty())
        throw std::invalid_argument(__FUNCTION__ ": empty id!");

    auto const it = std::find_if(container.begin(), container.end(),
        [id](T::const_reference e) { return e.id() == id; });
    if (it != container.end())
        return &*it;

    BOOST_FOREACH (EntityCollection& ec, sub) {
        if (auto e = getsub<typename T::pointer>(ec, id))
            return e;
    }

    return nullptr;
}

EntityCollection::~EntityCollection()
{
    // NOP
}

Entity* EntityCollection::operator[] (std::string const& id)
{
    return findEntityOrCollection(m_entities, m_collections, id);
}

EntityCollection* EntityCollection::collection(std::string const& id)
{
    return findEntityOrCollection(m_collections, m_collections, id);
}

void EntityCollection::clear()
{
    BOOST_FOREACH (Entity& e, m_entities)
        e.kill();
    m_entities.clear();

    BOOST_FOREACH (EntityCollection& ec, m_collections)
        ec.clear();
    m_collections.clear();
}

void EntityCollection::tidy()
{
    m_entities.erase_if([](Entity const& e) {
        return e.state() == Entity::killed;
    });

    m_collections.erase_if([](EntityCollection const& ec) {
        return ec.killed();
    });

    BOOST_FOREACH (EntityCollection& ec, m_collections)
        ec.tidy();
}

void EntityCollection::add(Entity& e)
{
    if (e.m_parent) {
        if (e.m_parent == this) {
            assert(!"Entity already added");
            return;
        }
        throw std::invalid_argument(
            "cannot add Entity: it already has a parent");
    }
    assert(e.id().empty() || (*this)[e.id()]); // double IDs *should* be avoided
    e.m_parent = this;
    m_entities.push_back(&e);
}

void EntityCollection::add(EntityCollection& ec)
{
    if (ec.m_parent) {
        if (ec.m_parent == this) {
            assert(!"EntityCollection already added");
            return;
        }
        throw std::invalid_argument(
            "cannot add EntityCollection: it already has a parent");
    }
    assert(ec.id().empty() || !collection(ec.id())); // double IDs *should* be avoided
    ec.m_parent = this;
    m_collections.push_back(&ec);
}
