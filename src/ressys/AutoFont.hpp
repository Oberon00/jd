// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef AUTO_FONT_HPP_INCLUDED
#define AUTO_FONT_HPP_INCLUDED AUTO_FONT_HPP_INCLUDED

#include "AutoResource.hpp"

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Text.hpp>


inline void updateAutoResourceMedia(sf::Text& t, ResourceTraits<sf::Font>::Ptr fnt)
{
    t.setFont(*fnt);
}


#endif //AUTO_FONT_HPP_INCLUDED
