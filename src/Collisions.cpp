#include "Collisions.hpp"
#include "compsys/BasicMetaComponent.hpp"
#include <boost/foreach.hpp>
#include <algorithm>
#include "Logfile.hpp"

JD_BASIC_COMPONENT_IMPL(CollisionManager)

CollisionManager::CollisionManager():
    m_pairsModified(false)
{
}

void CollisionManager::addGroup(std::string const& id, CollideableGroup& g)
{
    if (!m_groups.insert(std::make_pair(id, &g)).second)
        throw Error(__FUNCTION__ ": double id");
}

static void removePaired(
    boost::container::flat_multimap<std::string, CollideableGroup*> pairs,
    CollideableGroup* g)
{
    for (auto it = pairs.begin(); it != pairs.end(); ) {
        if (it->second == g)
            it = pairs.erase(it);
        else
            ++it;
    }
}

void CollisionManager::removeGroup(std::string const& id)
{
    auto const it = m_groups.find(id);
    if (it == m_groups.end())
        throw Error("trying to remove a group, which was not registered.");

    m_pairsModified = true;
    auto rng = m_sPairs.equal_range(id);
    m_sPairs.erase(rng.first, rng.second);
    rng = m_sReversePairs.equal_range(id);
    m_sReversePairs.erase(rng.first, rng.second);
    removePaired(m_sPairs, it->second);
    removePaired(m_sReversePairs, it->second);
    
    m_groups.erase(it);
}

void CollisionManager::addPairing(std::string const& aId, std::string const& bId)
{
    auto const iA = m_groups.find(aId);
    if (iA == m_groups.end())
        throw Error(__FUNCTION__ ": invalid aId");

    auto const iB = m_groups.find(bId);
    if (iB == m_groups.end())
        throw Error(__FUNCTION__ ": invalid bId");
    m_sPairs.insert(std::make_pair(aId, iB->second));
    m_sReversePairs.insert(std::make_pair(bId, iA->second));
    m_pairsModified = true;
}

CollideableGroup* CollisionManager::operator[] (std::string const& id)
{
    auto const it = m_groups.find(id);
    return it == m_groups.end() ? nullptr : it->second;
}

inline CollideableGroup* nonNull(CollideableGroup* g)
{
    assert(g);
    return g;
}

void CollisionManager::updatePairings()
{
    if (!m_pairsModified)
        return;

    m_pairs.clear();
    BOOST_FOREACH (auto const& sPair, m_sPairs)
        m_pairs.push_back(std::make_pair(
            nonNull(m_groups[sPair.first]),
            nonNull(sPair.second)));

    m_pairsModified = false;
}

void CollisionManager::collide()
{
    updatePairings();
    BOOST_FOREACH (auto const& pair, m_pairs) {
        pair.first->collideWith(*pair.second);
    }
}

template <typename Range>
std::vector<Collision> getColliding(Range const& range, sf::FloatRect const& rect)
{
    std::vector<Collision> result;
    BOOST_FOREACH(auto const& group, range) {
        auto const colliding = group.second->colliding(rect);
        result.insert(colliding.end(), colliding.begin(), colliding.end());
    }
    return result;
}

template <typename Range>
std::vector<Collision> getColliding(Range const& range, sf::Vector2f p1, sf::Vector2f p2)
{
    std::vector<Collision> result;
    BOOST_FOREACH(auto const& group, range) {
        auto const colliding = group.second->colliding(p1, p2);
        result.insert(colliding.end(), colliding.begin(), colliding.end());
    }
    return result;
}

std::vector<Collision> CollisionManager::colliding(sf::FloatRect const& r)
{
    return getColliding(m_groups, r);
}

std::vector<Collision> CollisionManager::colliding(sf::FloatRect const& r, std::string const& aId)
{
    auto a(getColliding(m_sPairs.equal_range(aId), r));
    auto b(getColliding(m_sReversePairs.equal_range(aId), r));

    a.insert(a.end(), b.begin(), b.end());
    return a;
}

std::vector<Collision> CollisionManager::colliding(
    sf::Vector2f lineStart, sf::Vector2f lineEnd)
{
    return getColliding(m_groups, lineStart, lineEnd);
}

std::vector<Collision> CollisionManager::colliding(
    sf::Vector2f lineStart, sf::Vector2f lineEnd, std::string const& aId)
{
    auto a(getColliding(m_sPairs.equal_range(aId), lineStart, lineEnd));
    auto b(getColliding(m_sReversePairs.equal_range(aId), lineStart, lineEnd));

    a.insert(a.end(), b.begin(), b.end());
    return a;
}


CollideableGroupRemover::~CollideableGroupRemover()
{
    try { m_manager.removeGroup(m_id); }
    catch (std::exception const& e) { LOG_EX(e); }
}
