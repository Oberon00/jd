// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef TILEMAP_HPP_INCLUDED
#define TILEMAP_HPP_INCLUDED TILEMAP_HPP_INCLUDED

#include "ressys/resfwd.hpp"
#include "sfUtil.hpp"

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>

#include <unordered_map>
#include <vector>

namespace jd {

typedef sf::Vector3<unsigned> Vector3u;

/* const */ class Tileset
{
public:
    enum class TileOrdering { linewiseOrdered, columnwiseOrdered };

    Tileset();
    Tileset(
        sf::Vector2u size,
        ResourceTraits<sf::Texture>::Ptr texture,
        TileOrdering ordering = TileOrdering::linewiseOrdered);

    sf::Vector2u position(unsigned index) const;
    ResourceTraits<sf::Texture>::Ptr texture() const { return m_texture; }
    sf::Vector2u size() const { return m_size; } // size of a single tile

private:
    sf::Vector2u m_size; // size of a single tile
    ResourceTraits<sf::Texture>::Ptr m_texture;
    TileOrdering m_ordering;
    sf::Vector2u m_tileCount;
};

// z-axis of a Vector3 is the layer.
class Tilemap: public sf::Drawable, public sf::Transformable
{
public:
    void setTileset(Tileset const& ts);
    Tileset const& tileset() const;

    void setSize(Vector3u const& size);
    Vector3u size() const;

    // get prefix for consistence with other drawables
    sf::FloatRect getLocalBounds() const;
    sf::FloatRect getGlobalBounds() const;

    unsigned operator[] (Vector3u pos) const;
    void set(Vector3u pos, unsigned tileId);

    sf::Vector2f localTilePos(sf::Vector2i pos) const;
    sf::Vector2f globalTilePos(sf::Vector2i pos) const;

    sf::FloatRect localTileRect(sf::Vector2i pos) const;
    sf::FloatRect globalTileRect(sf::Vector2i pos) const;

    sf::Vector2i tilePosFromLocal(sf::Vector2f pos) const;
    sf::Vector2i tilePosFromGlobal(sf::Vector2f pos) const;

    bool isValidPosition(sf::Vector3i pos) const;

    void addAnimation(std::size_t tid, std::size_t lastTid, float speed);
    void addAnimation(Vector3u pos, std::size_t lastTid, float speed);
    void removeAnimation(std::size_t tid);
    void removeAnimation(Vector3u pos);

    void animate(sf::Time elapsedTime);

protected:
    virtual void draw(
        sf::RenderTarget& target, sf::RenderStates states
    ) const;

private:
    class Animation {
    public:
        explicit Animation(
            std::size_t firstTid = 0, std::size_t lastTid = 0, float speed = 0.f
        ):
            m_firstTid(firstTid), m_lastTid(lastTid),
            m_speed(speed), m_currentFrame(static_cast<float>(firstTid))
        { }

        void animate(sf::Time elapsedTime);
        std::size_t currentFrame() const;

    private:
        std::size_t m_firstTid;
        std::size_t m_lastTid;
        float m_speed; // in frames per second
        float m_currentFrame;
    };
    std::size_t maybeAnimated(std::size_t tid, Vector3u pos) const;

    std::size_t index(Vector3u pos) const;

    std::unordered_map<std::size_t, Animation> m_tidAnimations;
    std::unordered_map<Vector3u, Animation> m_posAnimations;

    Tileset m_tileset;

    std::size_t m_columnCount;
    std::size_t m_rowCount;
    std::vector<unsigned> m_map;
};

} // namespace jd


#endif
