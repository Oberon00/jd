// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef TILE_COLLIDEABLE_GROUP_HPP_INCLUDED
#define TILE_COLLIDEABLE_GROUP_HPP_INCLUDED TILE_COLLIDEABLE_GROUP_HPP_INCLUDED

#include "Collisions.hpp"
#include "sfUtil.hpp" // std::hash<sf::Vector2<T>>
#include "WeakRef.hpp"

#include <unordered_map>


class TileCollisionComponent;

namespace jd { class Tilemap; }
typedef sf::Vector3<unsigned> Vector3u;

class TileCollideableInfo: public EnableWeakRefFromThis<TileCollideableInfo> {
public:
    TileCollideableInfo(jd::Tilemap& tilemap);

    void setProxy(unsigned tileId, TileCollisionComponent* proxy);
    WeakRef<TileCollisionComponent> proxy(unsigned tileId);

    void setColliding(Vector3u pos, TileCollisionComponent* c);
    WeakRef<TileCollisionComponent> colliding(Vector3u pos);

    // notify if e != nullptr
    std::vector<Collision> colliding(
        sf::FloatRect const&,
        Entity* e = nullptr,
        std::vector<Vector3u>* positions = nullptr);

    std::vector<Collision> colliding(
        sf::Vector2f lineStart,
        sf::Vector2f lineEnd,
        std::vector<Vector3u>* positions = nullptr);

    jd::Tilemap& tilemap() { return m_tilemap; }
    Vector3u mapsize() const;

    void clear() { m_entities.clear(); m_proxyEntities.clear(); }

    Collision makeCollision(
        Vector3u pos,
        Entity* notified = nullptr,
        sf::FloatRect const& foreignRect = sf::FloatRect());

    bool clipToMap(
        sf::Vector2f lineStart, sf::Vector2f lineEnd,
        sf::Vector2u& clipStart, sf::Vector2u& clipEnd);
    sf::Vector2u findNext(
        sf::Vector2u pos, sf::Vector2u oldPos,
        sf::Vector2u lineStart, sf::Vector2u lineEnd,
        bool* found = nullptr);
    std::size_t mapCorners(
        sf::FloatRect const& r,
        sf::Vector2u& begin,
        sf::Vector2u& last);
private:
    TileCollideableInfo& operator= (TileCollideableInfo const&);

    std::unordered_map<Vector3u, WeakRef<TileCollisionComponent>> m_entities;
    std::unordered_map<unsigned, WeakRef<TileCollisionComponent>> m_proxyEntities;
    jd::Tilemap& m_tilemap;
};

class TileLayersCollideableGroup: public CollideableGroup {
public:
    TileLayersCollideableGroup(
        TileCollideableInfo* data,
        unsigned firstLayer, unsigned endLayer);

    WeakRef<TileCollideableInfo> data() { return m_data; }

    unsigned firstLayer() const { return m_firstLayer; }
    void setFirstLayer(unsigned layer);
    unsigned endLayer() const { return m_endLayer; }
    void setEndLayer(unsigned layer);


    virtual std::vector<Collision> colliding(
        sf::FloatRect const&, Entity* e = nullptr) override;

    virtual std::vector<Collision> colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd) override;

    virtual void clear()  override { m_firstLayer = m_endLayer; }

private:
    bool layerInRange(unsigned layer) const
    {
        return layer >= m_firstLayer && layer < m_endLayer;
    }

    bool isUnfiltered() const;

    WeakRef<TileCollideableInfo> m_data;
    unsigned m_firstLayer;
    unsigned m_endLayer;
};

class TileStackCollideableGroup: public CollideableGroup {
public:
    struct Info {
        Info(): tileId(0), discard(true) { }
        Info(unsigned tileId, WeakRef<Entity> const& entity):
            tileId(tileId), entity(entity), discard(false) { }

        unsigned tileId;
        WeakRef<Entity> entity;
        bool discard;
    };

    typedef std::function<void(sf::Vector2u, std::vector<Info>&)> FilterCallback;

    TileStackCollideableGroup(
        WeakRef<TileCollideableInfo> const& data,
        FilterCallback filter = FilterCallback()
    ):
        m_data(data),
        m_filter(filter)
    { }

    virtual std::vector<Collision> colliding(
        sf::FloatRect const&, Entity* e = nullptr) override;

    virtual std::vector<Collision> colliding(
        sf::Vector2f lineStart, sf::Vector2f lineEnd) override;

    virtual void clear() override { m_filter = FilterCallback(); }

    WeakRef<TileCollideableInfo> data() { return m_data; }

    FilterCallback setFilter(FilterCallback filter);

private:
    void processStack(
        std::vector<Info>& stack, std::vector<Collision>& result,
        sf::FloatRect const& r) const;

    WeakRef<TileCollideableInfo> m_data;
    FilterCallback m_filter;
};

#endif
