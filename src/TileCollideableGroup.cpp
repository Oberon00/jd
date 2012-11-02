#include "TileCollideableGroup.hpp"
#include "Tilemap.hpp"
#include "comp/TileCollisionComponent.hpp"
#include "Logfile.hpp"
#include <unordered_set>
#include <array>
#include <algorithm>
#include <functional>
#include <compsys/Entity.hpp>

TileCollideableInfo::TileCollideableInfo(jd::Tilemap& tilemap):
    m_tilemap(tilemap)
{
}

void TileCollideableInfo::setProxy(unsigned tileId, TileCollisionComponent* proxy)
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

TileCollisionComponent* TileCollideableInfo::proxy(unsigned tileId)
{
    auto const it = m_proxyEntities.find(tileId);
    if (it != m_proxyEntities.end())
        return it->second.get();
    return nullptr;
}

void TileCollideableInfo::setColliding(Vector3u pos, TileCollisionComponent* e)
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

TileCollisionComponent* TileCollideableInfo::colliding(Vector3u pos)
{
    auto const it = m_entities.find(pos);
    if (it != m_entities.end())
        return it->second.get();
    return nullptr;
}

Vector3u TileCollideableInfo::mapsize() const
{
    return m_tilemap.size();
}

Collision TileCollideableInfo::makeCollision(
    Vector3u pos, Entity* e, sf::FloatRect const& r)
{
    auto const iEntity = m_entities.find(pos);
    if (iEntity != m_entities.end()) {
        if (e)
            iEntity->second->notifyCollision(pos, *e, r);
         return Collision(
            iEntity->second->parent(),
            m_tilemap.globalTileRect(sf::Vector2i(pos.x, pos.y)));
    } else {
        unsigned const tileId = m_tilemap[pos];
        auto const iProxy = m_proxyEntities.find(tileId);
        if (iProxy != m_proxyEntities.end()) {
            if (e)
                iProxy->second->notifyCollision(pos, *e, r);
            return Collision(
                iProxy->second->parent(),
                m_tilemap.globalTileRect(sf::Vector2i(pos.x, pos.y)));
        }
    } // if no entity at pos registered
    return Collision();
}

std::vector<Collision> TileCollideableInfo::colliding(
    sf::FloatRect const& r, Entity* e, std::vector<Vector3u>* positions)
{
    sf::Vector2u begin;
    sf::Vector2u last;
    std::size_t const intersectingCount = mapCorners(r, begin, last);


    std::vector<Collision> result;
    if (intersectingCount <= 0)
        return result;
    result.reserve(intersectingCount);

    Vector3u pos;
    for (pos.z = 0; pos.z < m_tilemap.size().z; ++pos.z) {
        for (pos.x = begin.x; pos.x <= last.x; ++pos.x) {
            for (pos.y = begin.y; pos.y <= last.y; ++pos.y) {
                auto const c = makeCollision(pos, e, r);
                if (c.entity) {
                    if (positions)
                        positions->push_back(pos);
                    result.push_back(c);
                }
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

std::vector<Collision> TileCollideableInfo::colliding(
    sf::Vector2f gp1, sf::Vector2f gp2, std::vector<Vector3u>* positions)
{
    sf::Vector2u p1;
    sf::Vector2u p2;

    std::vector<Collision> result;

    if (!clipToMap(gp1, gp2, p1, p2))
        return result;

    result.reserve(static_cast<std::size_t>(
        jd::math::abs(static_cast<sf::Vector2f>(p2 - p1)) * mapsize().z));

    sf::Vector2u pos = p1;
    sf::Vector2u lastPos = p1;
    for (;;) {
        Vector3u idx(pos.x, pos.y, 0);
        for (; idx.z < m_tilemap.size().z; ++idx.z) {
            auto const c = makeCollision(idx, nullptr, sf::FloatRect());
            if (c.entity) {
                if (positions)
                    positions->push_back(idx);
                result.push_back(c);
            }
        }
        bool found;
        sf::Vector2u const nextPos = findNext(pos, lastPos, p1, p2, &found);
        if (!found)
            break;

        lastPos = pos;
        pos = nextPos;
    }
    return result;
}

bool TileCollideableInfo::clipToMap(
    sf::Vector2f lineStart, sf::Vector2f lineEnd,
    sf::Vector2u& clipStart, sf::Vector2u& clipEnd)
{
    clipStart = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(lineStart));
    clipEnd = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(lineEnd));

    if (!jd::clipLine(clipStart, clipEnd, sf::Rect<unsigned>(
        0, 0, m_tilemap.size().x - 1, m_tilemap.size().y - 1)))
        return false;
    return true;
}

sf::Vector2u TileCollideableInfo::findNext(
    sf::Vector2u pos, sf::Vector2u oldPos_,
    sf::Vector2u lineStart, sf::Vector2u lineEnd,
    bool* found)
{
    sf::Vector2i const oldPos(oldPos_);
    auto const surrounding(surroundingTiles(static_cast<sf::Vector2i>(pos)));
    for (sf::Vector2i next : surrounding) {
        using jd::vec_cast;
        if (next != oldPos &&
            m_tilemap.isValidPosition(jd::vec2to3(next, 0)) &&
            jd::onLine(vec_cast<int>(lineStart), vec_cast<int>(lineEnd), next)
        ) {
            if (found)
                *found = true;
            return static_cast<sf::Vector2u>(next);
        }
    }
    if (found)
        *found = false;
    return pos;
}

std::size_t TileCollideableInfo::mapCorners(
    sf::FloatRect const& r,
    sf::Vector2u& begin,
    sf::Vector2u& last)
{
    begin = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(jd::topLeft(r)));
    begin.x = std::max(begin.x, 0u);
    begin.y = std::max(begin.y, 0u);
    last = static_cast<sf::Vector2u>(
        m_tilemap.tilePosFromGlobal(jd::bottomRightIn(r)));
    last.x = std::min(last.x, m_tilemap.size().x - 1);
    last.y = std::min(last.y, m_tilemap.size().y - 1);

    int intersectingPerLayer = (last.x - begin.x + 1) * (last.y - begin.y + 1);
    if (intersectingPerLayer <= 0) {
        return 0;
    }

    return intersectingPerLayer * m_tilemap.size().z;
}


template <typename Pred>
std::vector<Collision> filter(
        std::vector<Collision>&& collisions,
        std::vector<Vector3u>&& positions,
        Pred pred)
{
    std::size_t i = 0;
    while (i < collisions.size()) {
        if (!pred(positions[i].z)) {
            collisions.erase(collisions.begin() + i);
            positions.erase(positions.begin() + i);
        } else {
            ++i;
        }
    }
    return collisions;
}

TileLayersCollideableGroup::TileLayersCollideableGroup(
    TileCollideableInfo* data,
    unsigned firstLayer, unsigned endLayer):
    m_data(data),
    m_firstLayer(firstLayer),
    m_endLayer(endLayer)
{
    // must call setEndLayer before setFirstLayer
    // to check if map has enough layers
    setEndLayer(endLayer);
    setFirstLayer(firstLayer);
}
    


void TileLayersCollideableGroup::setFirstLayer(unsigned layer)
{
    if (layer >= m_endLayer)
        throw std::out_of_range("layer >= end-layer");
    assert(layer < m_data->mapsize().z);
    m_firstLayer = layer;
}

void TileLayersCollideableGroup::setEndLayer(unsigned layer)
{
    if (layer <= m_firstLayer)
        throw std::out_of_range("layer <= first layer");
    if (layer >= m_data->mapsize().z)
        throw std::out_of_range("layer >= count of layers");
    m_endLayer = layer;
}


std::vector<Collision> TileLayersCollideableGroup::colliding(
    sf::FloatRect const& r, Entity* e)
{
    if (m_firstLayer == m_endLayer) // was clear() called?
        return std::vector<Collision>();
    if (isUnfiltered())
        return m_data->colliding(r, e);

    std::vector<Vector3u> positions;
    return filter(
        m_data->colliding(r, e, &positions),
        std::move(positions),
        std::bind(&TileLayersCollideableGroup::layerInRange, this, std::placeholders::_1));
}

std::vector<Collision> TileLayersCollideableGroup::colliding(
    sf::Vector2f lineStart, sf::Vector2f lineEnd)
{
    if (m_firstLayer == m_endLayer) // was clear() called?
        return std::vector<Collision>();
    if (isUnfiltered())
        return m_data->colliding(lineStart, lineEnd);

    std::vector<Vector3u> positions;
    return filter(
        m_data->colliding(lineStart, lineEnd, &positions),
        std::move(positions),
        std::bind(&TileLayersCollideableGroup::layerInRange, this, std::placeholders::_1));
}

bool TileLayersCollideableGroup::isUnfiltered() const {
    return m_firstLayer == 0 && m_endLayer == m_data->mapsize().z;
}


std::vector<Collision> TileStackCollideableGroup::colliding(
    sf::FloatRect const& r, Entity* e)
{
    std::vector<Collision> result;

    if (!m_filter)
        return result;

    sf::Vector2u begin;
    sf::Vector2u last;
    std::size_t const intersectingCount = m_data->mapCorners(r, begin, last);

    if (intersectingCount <= 0)
        return result;
    result.reserve(intersectingCount);

    Vector3u pos;
    for (pos.x = begin.x; pos.x <= last.x; ++pos.x) {
        for (pos.y = begin.y; pos.y <= last.y; ++pos.y) {
            std::vector<Info> stack;
            auto const pos2 = jd::vec3to2(pos);

            for (pos.z = 0; pos.z < m_data->mapsize().z; ++pos.z) {
                auto c = m_data->makeCollision(pos, e, r);
                if (c.entity)
                    stack.emplace_back(m_data->tilemap()[pos], c.entity);
                else
                    stack.emplace_back();
            } // for z
            m_filter(pos2, stack);
            processStack(
                stack,
                result,
                m_data->tilemap().globalTileRect(sf::Vector2i(pos2)));
        } // for y
    } // for x
    return result;
}


void TileStackCollideableGroup::processStack(
    std::vector<Info>& stack, std::vector<Collision>& result,
    sf::FloatRect const& r) const
{
    for (auto const& info: stack) {
        if (!info.discard && info.entity.valid())
            result.emplace_back(info.entity.getOpt(), r);
    } // for info: stack
}


std::vector<Collision> TileStackCollideableGroup::colliding(
    sf::Vector2f gp1, sf::Vector2f gp2)
{
    std::vector<Collision> result;

    if (!m_filter)
        return result;

    sf::Vector2u p1;
    sf::Vector2u p2;
    if (!m_data->clipToMap(gp1, gp2, p1, p2))
        return result;

    result.reserve(static_cast<std::size_t>(
        jd::math::abs(
            jd::vec_cast<float>(p2) - jd::vec_cast<float>(p1)
        ) * m_data->mapsize().z));

    sf::Vector2u pos = p1;
    sf::Vector2u lastPos = p1;
    for (;;) {
        std::vector<Info> stack;
        Vector3u idx(pos.x, pos.y, 0);
        for (; idx.z < m_data->mapsize().z; ++idx.z) {
            auto const c = m_data->makeCollision(idx, nullptr, sf::FloatRect());
            if (c.entity)
                stack.emplace_back(m_data->tilemap()[idx], c.entity);
            else
                stack.emplace_back();
        }
        m_filter(pos, stack);
        processStack(
            stack, result, m_data->tilemap().globalTileRect(sf::Vector2i(pos)));

        bool found;
        sf::Vector2u const nextPos = m_data->findNext(pos, lastPos, p1, p2, &found);
        if (!found)
            break;
        lastPos = pos;
        pos = nextPos;
    }
    return result;
}

    
    TileStackCollideableGroup::FilterCallback
TileStackCollideableGroup::setFilter(FilterCallback filter)
{
    auto oldfilter = m_filter;
    m_filter = filter;
    return oldfilter;
}


