#include "TilePositionComponent.hpp"

#include "compsys/Entity.hpp"
#include "PositionComponent.hpp"
#include "sfUtil.hpp"
#include "Tilemap.hpp"

#include <boost/bind.hpp>


TilePositionComponent::TilePositionComponent(jd::Tilemap const& tilemap, unsigned layer):
    m_tilePosition(0, 0, layer),
    m_tilemap(tilemap)
{
}

TilePositionComponent::TilePositionComponent(
    Entity& parent, jd::Tilemap const& tilemap, unsigned layer):
    m_tilePosition(0, 0, layer),
    m_tilemap(tilemap)
{
    parent.add(*this);
}



void TilePositionComponent::cleanupComponent()
{
    m_con_positionChanged.disconnect();
}

void TilePositionComponent::initComponent()
{
    assert(parent());
    PositionComponent& p = parent()->require<PositionComponent>();
    m_con_positionChanged = p.connect_rectChanged(
        boost::bind(&TilePositionComponent::on_positionChanged, this, _1, _2));
    on_positionChanged(p.rect(), p.rect());
}

void TilePositionComponent::on_positionChanged(
        sf::FloatRect const& oldRect, sf::FloatRect const& newRect)
{
    assert(
        oldRect == newRect ||
        m_tilemap.tilePosFromGlobal(jd::center(oldRect)) ==
            static_cast<sf::Vector2i>(jd::vec3to2(m_tilePosition)));

    sf::Vector3i const newPos = jd::vec2to3(
        m_tilemap.tilePosFromGlobal(jd::center(newRect)),
        static_cast<int>(m_tilePosition.z));
    if (!m_tilemap.isValidPosition(newPos))
        throw std::runtime_error(
            "TilePositionComponent must always have a valid "
            "position for the corresponding Tilemap");

    Vector3u const oldPos = m_tilePosition;
    m_tilePosition = static_cast<Vector3u>(newPos);
    m_sig_tilePositionChanged(oldPos, m_tilePosition);
}

