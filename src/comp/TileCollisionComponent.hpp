#ifndef TILE_COLLISION_COMPONENT_HPP_INCLUDED
#define TILE_COLLISION_COMPONENT_HPP_INCLUDED TILE_COLLISION_COMPONENT_HPP_INCLUDED

#include "compsys/Component.hpp"
#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector3.hpp>
#include <WeakRef.hpp>
#include <ssig.hpp>

class TileCollideableInfo;

typedef sf::Vector3<unsigned> Vector3u;

class TileCollisionComponent: public Component {
    JD_COMPONENT

    SSIG_DEFINE_MEMBERSIGNAL(collided, void(Vector3u, Entity&, sf::FloatRect))
    SSIG_DEFINE_MEMBERSIGNAL(overridden, void(Vector3u, TileCollisionComponent&))

public:
    explicit TileCollisionComponent(Entity& parent);
    explicit TileCollisionComponent(TileCollideableInfo& tileinfo);
    TileCollisionComponent(Entity& parent, TileCollideableInfo& tileinfo);

    virtual void initComponent();
    virtual void cleanupComponent();


    void notifyCollision(Vector3u p, Entity& c, sf::FloatRect cr)
    {
        m_sig_collided(p, c, cr);
    }

    void notifyOverride(Vector3u p, TileCollisionComponent& c)
    {
        m_sig_overridden(p, c);
    }

    TileCollideableInfo const* tileInfo() const { return m_tileinfo.getOpt(); }

private:
    void on_tilePositionChanged(
        sf::Vector3<unsigned> oldPos, sf::Vector3<unsigned> newPos);

    ScopedConnection<
        void(sf::Vector3<unsigned>, sf::Vector3<unsigned>)
    > m_con_positionChanged;
    WeakRef<TileCollideableInfo> m_tileinfo;
};

#endif
