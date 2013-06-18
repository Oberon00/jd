#ifndef VFILE_FONT_HPP_INCLUDED
#define VFILE_FONT_HPP_INCLUDED VFILE_FONT_HPP_INCLUDED

#include "svc/FileSystem.hpp"

#include <SFML/Graphics/Font.hpp>

static void loadFontResource(struct VFileFont&, std::string const&);

struct VFileFont: public sf::Font
{
private:
    friend void loadFontResource(VFileFont&, std::string const&);
    VFile stream;
};

#endif //VFILE_FONT_HPP_INCLUDED
