#ifndef TILEPOSITION_HPP_INCLUDED
#define TILEPOSITION_HPP_INCLUDED TILEPOSITION_HPP_INCLUDED

#include "compsys/Component.hpp"

#include "ssig.hpp"

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Vector3.hpp>
#include <string>


namespace jd { class Tilemap; }
typedef sf::Vector3<unsigned> Vector3u;



class TilePositionComponent: public Component {
    JD_COMPONENT

    SSIG_DEFINE_MEMBERSIGNAL(tilePositionChanged,
        void(Vector3u oldPos, Vector3u newPos))
public:
    explicit TilePositionComponent(jd::Tilemap const& tilemap, unsigned layer = 0);
    TilePositionComponent(
        Entity& parent, jd::Tilemap const& tilemap, unsigned layer = 0);

    virtual void initComponent();
    virtual void cleanupComponent();

    Vector3u tilePosition() const { return m_tilePosition; }

private:
    void on_positionChanged(
        sf::FloatRect const& oldRect, sf::FloatRect const& newRect);

    ScopedConnection<void(sf::FloatRect const&, sf::FloatRect const&)>
        m_con_positionChanged;
    Vector3u m_tilePosition;
    jd::Tilemap const& m_tilemap;

};

#endif
