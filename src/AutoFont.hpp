#ifndef AUTO_FONT_HPP_INCLUDED
#define AUTO_FONT_HPP_INCLUDED AUTO_FONT_HPP_INCLUDED

#include "AutoResource.hpp"
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Font.hpp>

inline void updateAutoResourceMedia(sf::Text& t, ResourceTraits<sf::Font>::Ptr fnt)
{
    t.setFont(*fnt);
}


#endif //AUTO_FONT_HPP_INCLUDED
