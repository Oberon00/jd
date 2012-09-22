#include "TileCollideableGroup.hpp"
#include "Tilemap.hpp"
#include "comp/TileCollisionComponent.hpp"
#include "Logfile.hpp"
#include <unordered_set>
#include <array>
#include <algorithm>

TileCollideableGroup::TileCollideableGroup(jd::Tilemap& tilemap):
    m_tilemap(tilemap)
{
}

void TileCollideableGroup::setProxy(unsigned tileId, TileCollisionComponent* proxy)
{
    if (!proxy) {
        std::size_t const erasedCount = m_proxyEntities.erase(tileId);
        assert(erasedCount < 2);
        if (!erasedCount)
            LOG_W("Attempt to unset a proxy, which was not set.");
    } else if (!m_proxyEntities.insert(std::make_pair(tileId, proxy)).second)
        throw std::logic_error(
            __FUNCTION__ ": cannot assign proxy entity: already assigned");
}

void TileCollideableGroup::setColliding(Vector3u pos, TileCollisionComponent* e)
{
    auto const it = m_entities.find(pos);
    bool const found = it != m_entities.end();
    if (e) {
        if (found) {
            it->second->notifyOverride(pos, *e);
            it->second = e;
        } else {
            bool const success = m_entities.insert(std::make_pair(pos, e)).second;
            assert(success);
        }
    } else if (found) {
        m_entities.erase(pos);
    } else {
        LOG_W("Attempt to unregister a TileCollisionComponent which was not registered.");
    }
}

void TileCollideableGroup::addCollisions(
    Vector3u pos, std::vector<Collision>& result,
    Entity* e, sf::FloatRect const& r)
{
    auto const iEntity = m_entities.find(pos);
    if (iEntity != m_entities.end()) {
        if (e)
            iEntity->second->notifyCollision(pos, *e, r);
        result.push_back(Collision(
            iEntity->second->parent(),
            m_tilemap.globalTileRect(sf::Vector2i(pos.x, pos.y))));
    } else {
        unsigned const tileId = m_tilemap[pos];
        auto const iProxy = m_proxyEntities.find(tileId);
        if (iProxy != m_proxyEntities.end()) {
            if (e)
                iProxy->second->notifyCollision(pos, *e, r);
            result.push_back(Collision(
                iProxy->second->parent(),
                m_tilemap.globalTileRect(sf::Vector2i(pos.x, pos.y))));
        }
    } // if no entity at pos registered
}

std::vector<Collision> TileCollideableGroup::colliding(sf::FloatRect const& r, Entity* e)
{
    sf::Vector2u begin = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(jd::topLeft(r)));
    begin.x = std::max(begin.x, 0u);
    begin.y = std::max(begin.y, 0u);
    sf::Vector2u last = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(jd::bottomRight(r)));
    last.x = std::min(last.x, m_tilemap.size().x - 1);
    last.y = std::min(last.y, m_tilemap.size().y - 1);

    std::vector<Collision> result;
    result.reserve((last.x - begin.x + 1) * (last.y - begin.y + 1));

    for (unsigned z = 0; z < m_tilemap.size().z; ++z) {
        for (unsigned x = begin.x; x <= last.x; ++x) {
            for (unsigned y = begin.y; y <= last.y; ++y) {
                Vector3u const pos(x, y, z);
                addCollisions(pos, result, e, r);
            } // for y
        } // for x
    } // for z
    return result;
}

namespace {
    std::array<sf::Vector2i, 8> surroundingTiles(sf::Vector2i p)
    {
        std::array<sf::Vector2i, 8> result;
        std::fill(result.begin(), result.end(), p);

        // 012
        // 3p4
        // 567
        --result[0].x; --result[0].y;
                       --result[1].y;
        ++result[2].x; --result[2].y;
        --result[3].x;
        ++result[4].x;
        --result[5].x; ++result[5].y;
                       ++result[6].y;
        ++result[7].x; ++result[7].y;

        return result;
    }
}

std::vector<Collision> TileCollideableGroup::colliding(sf::Vector2f gp1, sf::Vector2f gp2)
{
    sf::Vector2u p1 = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(gp1));
    sf::Vector2u p2 = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(gp2));

    std::vector<Collision> result;

    if (!jd::clipLine(p1, p2, sf::Rect<unsigned>(
        0, 0, m_tilemap.size().x, m_tilemap.size().y)))
        return result;

    sf::Vector2u const d = p2 - p1;

    result.reserve(static_cast<std::size_t>(
        jd::math::abs(static_cast<sf::Vector2f>(d))));

    sf::Vector2u pos = p1;
    sf::Vector2u lastPos = pos;
    bool foundNext = false;
    do {
        Vector3u idx(pos.x, pos.y, 0);
        for (; idx.z < m_tilemap.size().z; ++idx.z)
            addCollisions(idx, result);

        auto const surrounding(surroundingTiles(
            static_cast<sf::Vector2i>(pos)));
        for (sf::Vector2i next : surrounding) {
            if (pos != lastPos &&
                m_tilemap.isValidPosition(jd::vec2to3(next, 0)) &&
                jd::intersection(
                    p1, p2,
                    sf::Rect<unsigned>(
                        static_cast<sf::Vector2u>(next),
                        sf::Vector2u(1, 1)))
            ) {
                lastPos = pos;
                pos = static_cast<sf::Vector2u>(next);
                foundNext = true;
                break;
            }
        }
        assert(foundNext);
    } while (pos != p2);
    return result;
}
