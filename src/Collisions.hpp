#ifndef COLLISIONS_HPP_INCLUDED
#define COLLISIONS_HPP_INCLUDED COLLISIONS_HPP_INCLUDED

#include <vector>
#include <SFML/Graphics/Rect.hpp>
#include <boost/container/flat_map.hpp>
#include <string>
#include "compsys/Component.hpp"

class Entity;

struct Collision {
    Collision(): entity(nullptr) { }
    Collision(Entity* e, sf::FloatRect r): entity(e), rect(r) { }
    Entity* entity;
    sf::FloatRect rect;
};

class CollideableGroup {
public:
    virtual ~CollideableGroup() { }

    // notify if e != nullptr
    virtual std::vector<Collision> const colliding(
        sf::FloatRect const&, Entity* e = nullptr) = 0;

    virtual std::vector<Collision> const colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd) = 0;

    // If a CollideableGroup delegates the collision check to the other one
    // it should call collideWith with delegated = isDelegated, to detect/avoid
    // endless recursion, if the other CollideableGroup also wants to delegate.
    enum DelegateState { notDelegated, isDelegated, reDelegated };
    virtual void collideWith(
        CollideableGroup& other, DelegateState delegated = notDelegated)
    { other.collideWith(*this, nextDelegateState(delegated)); }

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

class CollisionManager: public Component {
    JD_COMPONENT
public:
    class Error: public std::runtime_error {
        public: Error (char const* msg): std::runtime_error(msg) { }
    };

    CollisionManager();

    void addGroup(std::string const& id, CollideableGroup& g);
    void removeGroup(std::string const& id);

    void addPairing(std::string const& aId, std::string const& bId);

    CollideableGroup* operator[] (std::string const& id);
    void collide();

    std::vector<Collision> colliding(sf::FloatRect const&);

    // Note: collisions will be checked with all groups which are paired with aName
    // (hence the name aName, like in bName), not with the group aName itself (except if
    // the group is paired with itself).
    std::vector<Collision> colliding(sf::FloatRect const&, std::string const& aId);

    std::vector<Collision> colliding(sf::Vector2f lineStart, sf::Vector2f lineEnd);
    std::vector<Collision> colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd, std::string const& aId);


private:
    void updatePairings();

    boost::container::flat_map<std::string, CollideableGroup*> m_groups;

    boost::container::flat_multimap<std::string, CollideableGroup*> m_sPairs;
    boost::container::flat_multimap<std::string, CollideableGroup*> m_sReversePairs;

    std::vector<std::pair<CollideableGroup*, CollideableGroup*>> m_pairs;
    bool m_pairsModified;
};

class CollideableGroupRemover {
public:
    CollideableGroupRemover(std::string const groupId, CollisionManager& manager):
        m_id(groupId), m_manager(manager)
        { }
        ~CollideableGroupRemover();
        std::string const& id() const { return m_id; }
        CollisionManager& manager() { return m_manager; }
private:
    CollideableGroupRemover& operator=(CollideableGroupRemover const&);

    std::string const m_id;
    CollisionManager& m_manager;
};


#endif
