#ifndef VFILE_MUSIC_HPP_INCLUDED
#define VFILE_MUSIC_HPP_INCLUDED VFILE_MUSIC_HPP_INCLUDED

#include "svc/FileSystem.hpp"

#include <SFML/Audio/Music.hpp>


struct VFileMusic: public sf::Music
{
    VFile stream;
};

#endif //VFILE_MUSIC_HPP_INCLUDED
