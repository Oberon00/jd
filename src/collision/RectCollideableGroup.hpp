#ifndef RECT_COLLIDEABLE_GROUP_HPP_INCLUDED
#define RECT_COLLIDEABLE_GROUP_HPP_INCLUDED RECT_COLLIDEABLE_GROUP_HPP_INCLUDED

#include "Collisions.hpp"
#include "WeakRef.hpp"

#include <forward_list>
#include <list>


class PositionComponent;

class RectCollideableGroup: public CollideableGroup {
public:
    void add(PositionComponent& c);
    void remove(PositionComponent& c);

    virtual std::vector<Collision> colliding(
        sf::FloatRect const&, Entity* e = nullptr) override;
    virtual void collideWith(
        CollideableGroup& other, DelegateState delegated = notDelegated) override;

    virtual std::vector<Collision> colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd) override;

    virtual void collide() override;

    virtual void clear()  override { m_removed.clear(); m_items.clear(); }

private:
    void removePending();

    std::list<WeakRef<PositionComponent>> m_items;
    std::forward_list<WeakRef<PositionComponent>> m_removed;
};

#endif
