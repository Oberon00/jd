#include "Collisions.hpp"

#include "Logfile.hpp"

#include <algorithm>


void CollideableGroupGroup::add(CollideableGroup& g)
{
    m_groups.push_back(g.ref());
}

void CollideableGroupGroup::remove(CollideableGroup const& g)
{
    m_groups.erase(std::remove_if(
        m_groups.begin(), m_groups.end(),
        [&g] (WeakRef<CollideableGroup> const& e) {
            return !e.getOpt() || e.getOpt() == &g;
        }), m_groups.end());
}

template <typename Container, typename Container2>
void append(Container& c, Container2&& appended)
{
    c.insert(
        end(c),
        std::make_move_iterator(begin(appended)),
        std::make_move_iterator(end(appended)));
}

// notify if e != nullptr
std::vector<Collision> CollideableGroupGroup::colliding(
    sf::FloatRect const& r, Entity* e)
{
    std::vector<Collision> result;
    forEachGroup([&](WeakRef<CollideableGroup>& g) {
        append(result, g->colliding(r, e));
    });
    return result;
}

std::vector<Collision> CollideableGroupGroup::colliding(
    sf::Vector2f lineStart, sf::Vector2f lineEnd)
{
    std::vector<Collision> result;
    forEachGroup([&](WeakRef<CollideableGroup>& g) {
        append(result, g->colliding(lineStart, lineEnd));
    });
    return result;
}

void CollideableGroupGroup::collideWith(
    CollideableGroup& other, DelegateState delegated)
{
    forEachGroup([&](WeakRef<CollideableGroup>& g) {
        g->collideWith(other, delegated);
    });
}

void CollideableGroupGroup::collide()
{
    forEachGroup([](WeakRef<CollideableGroup>& g) {
        g->collide();
    });
}

// Removes all CollideableGroups from this CollideableGroupGroup, but does
// not clear them.
void CollideableGroupGroup::clear()
{
    m_groups.clear();
}

void CollideableGroupGroup::tidy()
{
   m_groups.erase(std::remove_if(
        m_groups.begin(), m_groups.end(),
        [] (WeakRef<CollideableGroup> const& e) {
            return !e.valid();
        }), m_groups.end());
}
