// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef VFILE_MUSIC_HPP_INCLUDED
#define VFILE_MUSIC_HPP_INCLUDED VFILE_MUSIC_HPP_INCLUDED

#include "svc/FileSystem.hpp"

#include <SFML/Audio/Music.hpp>


struct VFileMusic: public sf::Music
{
    VFile stream;
};

#endif //VFILE_MUSIC_HPP_INCLUDED
