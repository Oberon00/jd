#include "TileCollisionComponent.hpp"

#include "TileCollideableGroup.hpp"
#include "compsys/Entity.hpp"
#include "TilePositionComponent.hpp"
#include <boost/bind.hpp>
#include "Tilemap.hpp"

TileCollisionComponent::TileCollisionComponent(TileCollideableGroup& group):
    m_group(&group)
{ }

TileCollisionComponent::TileCollisionComponent(Entity& parent):
    m_group(nullptr)
{
    parent.add(*this);
}


TileCollisionComponent::TileCollisionComponent(
    Entity& parent, TileCollideableGroup& group):
    m_group(&group)
{
    parent.add(*this);
}

void TileCollisionComponent::cleanupComponent()
{
    if (!m_group)
        return;
    m_con_positionChanged.disconnect();
    m_group->setColliding(
        parent()->require<TilePositionComponent>().tilePosition(), nullptr);
}

void TileCollisionComponent::initComponent()
{
    assert(parent());
    if (!m_group)
        return;
    auto& p = parent()->require<TilePositionComponent>();
    m_con_positionChanged = p.connect_tilePositionChanged(
        boost::bind(&TileCollisionComponent::on_tilePositionChanged, this, _1, _2));
    on_tilePositionChanged(p.tilePosition(), p.tilePosition());
}


void TileCollisionComponent::on_tilePositionChanged(
    Vector3u oldPos, Vector3u newPos)
{
    if (!m_group)
        return;

    if (oldPos != newPos)
        m_group->setColliding(oldPos, nullptr);
    m_group->setColliding(newPos, this);
}
