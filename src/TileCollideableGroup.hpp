#ifndef TILE_COLLIDEABLE_GROUP_HPP_INCLUDED
#define TILE_COLLIDEABLE_GROUP_HPP_INCLUDED TILE_COLLIDEABLE_GROUP_HPP_INCLUDED

#include "Collisions.hpp"
#include "WeakRef.hpp"
#include <unordered_map>
#include "sfUtil.hpp" // std::hash<sf::Vector2<T>>

class TileCollisionComponent;

namespace jd { class Tilemap; }
typedef sf::Vector3<unsigned> Vector3u;

class TileCollideableGroup: public CollideableGroup {
public:
    TileCollideableGroup(jd::Tilemap& tilemap);

    void setProxy(unsigned tileId, TileCollisionComponent* proxy);
    void setColliding(Vector3u pos, TileCollisionComponent* c);

    // notify if e != nullptr
    virtual std::vector<Collision> const colliding(
        sf::FloatRect const&, Entity* e = nullptr);

    virtual std::vector<Collision> const colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd);

    jd::Tilemap& tilemap() { return m_tilemap; }

    virtual void clear() { m_entities.clear(); m_proxyEntities.clear(); }

private:
    void addCollisions(
        Vector3u pos,
        std::vector<Collision>& collisions,
        Entity* notified = nullptr, 
        sf::FloatRect const& foreignRect = sf::FloatRect());
    TileCollideableGroup& operator= (TileCollideableGroup const&);

    std::unordered_map<Vector3u, WeakRef<TileCollisionComponent>> m_entities;
    std::unordered_map<unsigned, WeakRef<TileCollisionComponent>> m_proxyEntities;
    jd::Tilemap& m_tilemap;
};

#endif