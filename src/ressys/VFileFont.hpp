// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef VFILE_FONT_HPP_INCLUDED
#define VFILE_FONT_HPP_INCLUDED VFILE_FONT_HPP_INCLUDED

#include "svc/FileSystem.hpp"

#include <SFML/Graphics/Font.hpp>

inline void loadFontResource(struct VFileFont& fnt, std::string const& name);

struct VFileFont: public sf::Font
{
private:
    friend void loadFontResource(VFileFont&, std::string const&);
    VFile stream;
};

#endif //VFILE_FONT_HPP_INCLUDED
