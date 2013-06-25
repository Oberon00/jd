// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "RectCollideableGroup.hpp"

#include "comp/PositionComponent.hpp"
#include "comp/RectCollisionComponent.hpp"
#include "compsys/Entity.hpp"
#include "sfUtil.hpp"


void RectCollideableGroup::add(PositionComponent& c)
{
    m_items.push_back(c.ref<PositionComponent>());
}


void RectCollideableGroup::remove(PositionComponent& c)
{
    m_removed.push_front(c.ref<PositionComponent>());
}



std::vector<Collision> RectCollideableGroup::colliding(
    sf::FloatRect const& r, Entity* e)
{
    removePending();

    std::vector<Collision> result;
    for (auto const& item : m_items) {
        if (!item->rect().intersects(r))
            continue;

        result.push_back(Collision(item->parent(), item->rect()));

        if (!e || !item->parent())
            continue;

        if (auto recv = item->parent()->get<RectCollisionComponent>())
            recv->notifyCollision(item->rect(), *e, r);

    } // foreach
    return result;
} // RectCollideableGroup::colliding


void RectCollideableGroup::collideWith(
    CollideableGroup& other, DelegateState)
{
    removePending();

    for (auto const& item : m_items) {
        std::vector<Collision> const collisions =
            other.colliding(item->rect(), item->parent());

        if (!item->parent())
            continue;

        auto recv = item->parent()->get<RectCollisionComponent>();

        if (!recv)
            continue;

        for (auto const& collision : collisions)
            recv->notifyCollision(
                item->rect(), *collision.entity, collision.rect);
    }
}

void RectCollideableGroup::removePending()
{
    while (!m_removed.empty()) {
        m_items.remove(m_removed.front());
        m_removed.pop_front();
    }
}

std::vector<Collision> RectCollideableGroup::colliding(
    sf::Vector2f p1, sf::Vector2f p2)
{
    removePending();

    std::vector<Collision> result;
    for (auto const& item : m_items) {
        if (jd::intersection(p1, p2, item->rect()))
            result.push_back(Collision(item->parent(), item->rect()));
    }
    return result;
}

static void notifyEntity(PositionComponent& p, PositionComponent& p2)
{
    auto recv = p.parent()->get<RectCollisionComponent>();
    if (recv)
        recv->notifyCollision(p.rect(), *p2.parent(), p2.rect());
}

void RectCollideableGroup::collide()
{
    removePending();

    for (auto it = begin(m_items); it != end(m_items); ++it) {
        PositionComponent& p = **it;
        for (auto it2 = std::next(it); it2 != end(m_items); ++it2) {
            PositionComponent& p2 = **it2;
            if (!p.rect().intersects(p2.rect()))
                continue;
            notifyEntity(p, p2);
            notifyEntity(p2, p);
        }
    }
}
