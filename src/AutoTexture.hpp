#ifndef AUTO_TEXTURE_HPP_INCLUDED
#define AUTO_TEXTURE_HPP_INCLUDED AUTO_TEXTURE_HPP_INCLUDED

#include "AutoResource.hpp"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Texture.hpp>

void updateAutoResourceMedia(sf::Sprite& s, ResourceTraits<sf::Texture>::Ptr tx)
{
    s.setTexture(*tx, true);
}

void updateAutoResourceMedia(sf::Shape& s, ResourceTraits<sf::Texture>::Ptr tx)
{
    s.setTexture(tx.get(), true);
}

#endif
