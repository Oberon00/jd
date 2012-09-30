#ifndef COLLISIONS_HPP_INCLUDED
#define COLLISIONS_HPP_INCLUDED COLLISIONS_HPP_INCLUDED

#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <string>
#include "WeakRef.hpp"

class Entity;

struct Collision {
    Collision(): entity(nullptr) { }
    Collision(Entity* e, sf::FloatRect r): entity(e), rect(r) { }
    Entity* entity;
    sf::FloatRect rect;
};

class CollideableGroup: public EnableWeakRefFromThis<CollideableGroup> {
public:
    virtual ~CollideableGroup() { }

    // notify if e != nullptr
    virtual std::vector<Collision> colliding(
        sf::FloatRect const&, Entity* e = nullptr) = 0;

    virtual std::vector<Collision> colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd) = 0;

    // If a CollideableGroup delegates the collision check to the other one
    // it should call collideWith with delegated = isDelegated, to detect/avoid
    // endless recursion, if the other CollideableGroup also wants to delegate.
    enum DelegateState { notDelegated, isDelegated, reDelegated };
    virtual void collideWith(
        CollideableGroup& other, DelegateState delegated = notDelegated)
    { other.collideWith(*this, nextDelegateState(delegated)); }

    virtual void collide() { }; // Check for internal collisions

    virtual void clear() = 0;

protected:
    static DelegateState nextDelegateState(DelegateState s)
    {
        int const result = static_cast<int>(s) + 1;
        if (s > reDelegated)
            throw std::logic_error("cannot delegate a redelegated collideWith() call");
        return static_cast<DelegateState>(result);
    }
};

class CollideableGroupGroup: public CollideableGroup {
public:

    // CollideableGroupGroup does not own g, but only holds a WeakRef to it
    void add(CollideableGroup& g);
    
    void remove(CollideableGroup const& g);

    // notify if e != nullptr
    virtual std::vector<Collision> colliding(
        sf::FloatRect const& r, Entity* e = nullptr) override;

    virtual std::vector<Collision> colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd) override;

    virtual void collideWith(
        CollideableGroup& other, DelegateState delegated = notDelegated) override;

    virtual void collide() override; // Check for internal collisions

    // Removes all CollideableGroups from this CollideableGroupGroup, but does
    // not clear them.
    virtual void clear() override;

private:
    template <typename F>
    void forEachGroup(F do_) {
        tidy();
        for (auto g: m_groups) {
            if (!g) {
                m_shouldTidy = true;
                continue;
            }
            do_(g);
        }
    }
    void tidy();

    std::vector<WeakRef<CollideableGroup>> m_groups;
    bool m_shouldTidy;
};



#endif
