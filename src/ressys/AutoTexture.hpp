// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef AUTO_TEXTURE_HPP_INCLUDED
#define AUTO_TEXTURE_HPP_INCLUDED AUTO_TEXTURE_HPP_INCLUDED

#include "AutoResource.hpp"

#include <SFML/Graphics/Shape.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>


inline void updateAutoResourceMedia(sf::Sprite& s, ResourceTraits<sf::Texture>::Ptr tx)
{
    s.setTexture(*tx, true);
}

inline void updateAutoResourceMedia(sf::Shape& s, ResourceTraits<sf::Texture>::Ptr tx)
{
    s.setTexture(tx.get(), true);
}

#endif
