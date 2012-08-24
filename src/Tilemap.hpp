#ifndef TILEMAP_HPP_INCLUDED
#define TILEMAP_HPP_INCLUDED TILEMAP_HPP_INCLUDED

#include <SFML/Graphics/Drawable.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/System/Vector2.hpp>
#include <SFML/System/Vector3.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <vector>
#include "resfwd.hpp"

namespace jd {

typedef sf::Vector3<unsigned> Vector3u;

/* const */ class Tileset
{
public:
    enum TileOrdering { linewiseOrdered, columnwiseOrdered };
    
    Tileset();
    Tileset(
        sf::Vector2u size,
        ResourceTraits<sf::Texture>::Ptr texture,
        TileOrdering ordering = linewiseOrdered);

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

protected:
    virtual void draw(
        sf::RenderTarget& target, sf::RenderStates states
    ) const;

private:
    std::size_t index(Vector3u pos) const;

    Tileset m_tileset;
    
    std::size_t m_columnCount;
    std::size_t m_rowCount;
    std::vector<unsigned> m_map;
};

} // namespace jd


#endif