// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#include "Tilemap.hpp"

#include "sfUtil.hpp"

#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/RenderTarget.hpp>

#include <cassert>
#include <iomanip>
#include <iostream>


namespace jd {

Tileset::Tileset():
    m_texture(nullptr),
    m_ordering(TileOrdering::linewiseOrdered)
{
}

Tileset::Tileset(
    sf::Vector2u size,
    ResourceTraits<sf::Texture>::Ptr tx,
    TileOrdering ordering
):
    m_size(size),
    m_texture(tx),
    m_ordering(ordering),
    m_tileCount(tx->getSize().x / size.x, tx->getSize().y / size.y)
{
    assert(ordering == TileOrdering::linewiseOrdered ||
           ordering == TileOrdering::columnwiseOrdered);
    assert(size.x <= m_texture->getSize().x);
    assert(size.y <= m_texture->getSize().y);
}

sf::Vector2u Tileset::position(unsigned index) const
{
    sf::Vector2u result;
    if (m_ordering == TileOrdering::linewiseOrdered) {
        result.x = m_size.x * (index % m_tileCount.x);
        result.y = m_size.y * (index / m_tileCount.x);
    } else {
        result.x = m_size.x * (index / m_tileCount.y);
        result.y = m_size.y * (index % m_tileCount.y);
    }
    return result;
}


//////////////////////////////////////////////////////////////////////////////////////////

void Tilemap::Animation::animate(sf::Time elapsedTime)
{
    m_currentFrame += elapsedTime.asSeconds() * m_speed;
    if (m_currentFrame > m_lastTid) {
        m_currentFrame = static_cast<float>(
            m_firstTid + std::fmod(
                m_currentFrame - m_firstTid, m_lastTid - m_firstTid));
    }
}

std::size_t Tilemap::Animation::currentFrame() const
{
    assert(m_currentFrame >= 0);
    return static_cast<std::size_t>(m_currentFrame);
}

//////////////////////////////////////////////////////////////////////////////////////////

void Tilemap::setTileset(Tileset const& ts)
{
    m_tileset = ts;
}

Tileset const& Tilemap::tileset() const
{
    return m_tileset;
}


void Tilemap::setSize(Vector3u const& size)
{
    m_map.resize(size.x * size.y * size.z);
    m_columnCount = size.x;
    m_rowCount = size.y;
}

Vector3u Tilemap::size() const
{
    return Vector3u(
        static_cast<unsigned>(m_columnCount),
        static_cast<unsigned>(m_rowCount),
        static_cast<unsigned>(m_map.size() / (m_columnCount * m_rowCount)));
}


// get prefix for consistence with other drawables
sf::FloatRect Tilemap::getLocalBounds() const
{
    return sf::FloatRect(
        0,
        0,
        static_cast<float>(m_columnCount * m_tileset.size().x),
        static_cast<float>(m_rowCount * m_tileset.size().y));
}

sf::FloatRect Tilemap::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


unsigned Tilemap::operator[] (Vector3u pos) const
{
    return m_map[index(pos)];
}

void Tilemap::set(Vector3u pos, unsigned tileId)
{
    m_map[index(pos)] = tileId;
}

std::size_t Tilemap::index(Vector3u pos) const
{
    return
        pos.z * m_rowCount * m_columnCount
      + pos.y * m_columnCount
      + pos.x;
}

sf::Vector2f Tilemap::localTilePos(sf::Vector2i pos) const
{
    return sf::Vector2f(
        static_cast<float>(pos.x * static_cast<int>(m_tileset.size().x)),
        static_cast<float>(pos.y * static_cast<int>(m_tileset.size().y)));
}

sf::Vector2f Tilemap::globalTilePos(sf::Vector2i pos) const
{
    return getTransform().transformPoint(localTilePos(pos));
}

sf::FloatRect Tilemap::localTileRect(sf::Vector2i pos) const
{
    return sf::FloatRect(
        localTilePos(pos),
        static_cast<sf::Vector2f>(m_tileset.size()));
}

sf::FloatRect Tilemap::globalTileRect(sf::Vector2i pos) const
{
    return getTransform().transformRect(localTileRect(pos));
}


sf::Vector2i Tilemap::tilePosFromLocal(sf::Vector2f pos) const
{
    return sf::Vector2i(
        static_cast<int>(pos.x / m_tileset.size().x),
        static_cast<int>(pos.y / m_tileset.size().y));
}

sf::Vector2i Tilemap::tilePosFromGlobal(sf::Vector2f pos) const
{
    return tilePosFromLocal(getInverseTransform().transformPoint(pos));
}

static void checkSpeed(float speed)
{
    if (speed < 0)
        throw std::out_of_range("negative animation speed not supported");
}

void Tilemap::addAnimation(std::size_t tid, std::size_t lastTid, float speed)
{
    checkSpeed(speed);
    m_tidAnimations[tid] = Animation(tid, lastTid, speed);
}

void Tilemap::addAnimation(Vector3u pos, std::size_t lastTid, float speed)
{
    checkSpeed(speed);
    m_posAnimations[pos] = Animation((*this)[pos], lastTid, speed);
}

void Tilemap::removeAnimation(std::size_t tid)
{
    m_tidAnimations.erase(tid);
}

void Tilemap::removeAnimation(Vector3u pos)
{
    m_posAnimations.erase(pos);
}


void Tilemap::animate(sf::Time elapsedTime)
{
    for (auto& a: m_tidAnimations)
        a.second.animate(elapsedTime);
    for (auto& a: m_posAnimations)
        a.second.animate(elapsedTime);

}


void Tilemap::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    sf::FloatRect const viewRect(jd::viewRect(target.getView()));

    Vector3u size = this->size();
    sf::Vector2i firstTPosI(tilePosFromGlobal(topLeft(viewRect)));
    firstTPosI.x = std::max(firstTPosI.x, 0);
    firstTPosI.y = std::max(firstTPosI.y, 0);
    sf::Vector2i lastTPosI(tilePosFromGlobal(bottomRight(viewRect)));
    lastTPosI.x = std::min(lastTPosI.x + 1, static_cast<int>(size.x));
    lastTPosI.y = std::min(lastTPosI.y + 1, static_cast<int>(size.y));
    sf::Vector2i distanceI = lastTPosI - firstTPosI;
    if (distanceI.x <= 0 || distanceI.y <= 0)
        return;
    
    auto distance = vec_cast<unsigned>(distanceI);
    
    auto firstTPos = vec_cast<unsigned>(firstTPosI);
    auto lastTPos = vec_cast<unsigned>(lastTPosI);
    
    std::size_t const renderedTileCount = distance.x * distance.y * size.z;
    static std::size_t const verticesPerTile = 4;
    std::vector<sf::Vertex> vertices(renderedTileCount * verticesPerTile);

    sf::Vector2f const firstPos(localTilePos(firstTPosI));
    std::size_t iVertices = 0;
    auto const tileSize = vec_cast<float>(m_tileset.size());
    std::size_t const mapSkipY = size.x - distance.x;
    std::size_t const mapSkipZ =
        size.x * size.y - distance.x * distance.y - mapSkipY * distance.y;

    std::size_t iMap = index(Vector3u(
        static_cast<unsigned>(firstTPos.x),
        static_cast<unsigned>(firstTPos.y),
        0));
    sf::Vector2f iPos(firstPos);
    for (unsigned z = 0; z < size.z; ++z) {
        for (unsigned y = firstTPos.y; y < static_cast<std::size_t>(lastTPos.y); ++y){
            for (unsigned x = firstTPos.x; x < static_cast<std::size_t>(lastTPos.x); ++x) {
                assert(index(Vector3u(x, y, z)) == iMap);
                unsigned const tileId = m_map[iMap++];
                if (tileId == 0) {
                    vertices.pop_back();
                    iPos.x += tileSize.x;
                    continue;
                }
                sf::Vector2f texPos(m_tileset.position(
                    static_cast<unsigned>(maybeAnimated(
                            tileId, Vector3u(x, y, z)) - 1)));
                vertices[iVertices].texCoords  = texPos;
                vertices[iVertices++].position = iPos;
                vertices[iVertices].texCoords  = sf::Vector2f(texPos.x, texPos.y + tileSize.y);
                vertices[iVertices++].position = sf::Vector2f(iPos.x,   iPos.y   + tileSize.y);
                vertices[iVertices].texCoords  = texPos + tileSize;
                vertices[iVertices++].position = iPos   + tileSize;
                vertices[iVertices].texCoords  = sf::Vector2f(texPos.x +  tileSize.x, texPos.y);
                vertices[iVertices++].position = sf::Vector2f(iPos.x   += tileSize.x,   iPos.y);
            } // for x
            iPos.x  = firstPos.x;
            iPos.y += tileSize.y;
            iMap   += mapSkipY;
        } // for y
        iPos.y = firstPos.y;
        iMap  += mapSkipZ;
    } // for z

    if (vertices.empty())
        return;
    states.transform *= getTransform();
    states.texture = m_tileset.texture().get();
    target.draw(
        &vertices[0],
        static_cast<unsigned>(vertices.size()),
                sf::Quads,
                states);
} // Tilemap::draw()

bool Tilemap::isValidPosition(sf::Vector3i pos) const
{
    sf::Vector3i const sz(size());
    return
        pos.x >= 0 && pos.y >= 0 && pos.z >= 0 &&
        pos.x < sz.x && pos.y < sz.y && pos.z < sz.z;
}

std::size_t Tilemap::maybeAnimated(std::size_t tid, Vector3u pos) const
{
    auto iPos = m_posAnimations.find(pos);
    if (iPos != m_posAnimations.end())
        return iPos->second.currentFrame();

    auto iTid = m_tidAnimations.find(tid);
    if (iTid != m_tidAnimations.end())
        return iTid->second.currentFrame();

    return tid;
}

} // namespace jd
