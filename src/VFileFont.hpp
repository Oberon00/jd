#ifndef VFILE_FONT_HPP_INCLUDED
#define VFILE_FONT_HPP_INCLUDED VFILE_FONT_HPP_INCLUDED

#include "svc/FileSystem.hpp"

#include <SFML/Graphics/Font.hpp>


struct VFileFont: public sf::Font
{
private:
    friend static void loadFontResource(VFileFont&, std::string const&);
    VFile stream;
};

#endif //VFILE_FONT_HPP_INCLUDED
