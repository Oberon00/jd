// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "TileCollisionComponent.hpp"

#include "collision/TileCollideableGroup.hpp"
#include "compsys/Entity.hpp"
#include "Tilemap.hpp"
#include "TilePositionComponent.hpp"

#include <boost/bind.hpp>


TileCollisionComponent::TileCollisionComponent(TileCollideableInfo& tileinfo):
    m_tileinfo(&tileinfo)
{ }

TileCollisionComponent::TileCollisionComponent(Entity& parent):
    m_tileinfo(nullptr)
{
    parent.add(*this);
}


TileCollisionComponent::TileCollisionComponent(
    Entity& parent, TileCollideableInfo& tileinfo):
    m_tileinfo(&tileinfo)
{
    parent.add(*this);
}

void TileCollisionComponent::cleanupComponent()
{
    if (!m_tileinfo)
        return;
    m_con_positionChanged.disconnect();
    m_tileinfo->setColliding(
        parent()->require<TilePositionComponent>().tilePosition(), nullptr);
}

void TileCollisionComponent::initComponent()
{
    assert(parent());
    if (!m_tileinfo)
        return;
    auto& p = parent()->require<TilePositionComponent>();
    m_con_positionChanged = p.connect_tilePositionChanged(
        boost::bind(&TileCollisionComponent::on_tilePositionChanged, this, _1, _2));
    on_tilePositionChanged(p.tilePosition(), p.tilePosition());
}


void TileCollisionComponent::on_tilePositionChanged(
    Vector3u oldPos, Vector3u newPos)
{
    if (!m_tileinfo)
        return;

    if (oldPos != newPos)
        m_tileinfo->setColliding(oldPos, nullptr);
    m_tileinfo->setColliding(newPos, this);
}
