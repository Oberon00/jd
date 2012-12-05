#ifndef VFILE_MUSIC_HPP_INCLUDED
#define VFILE_MUSIC_HPP_INCLUDED VFILE_MUSIC_HPP_INCLUDED

#include <SFML/Audio/Music.hpp>
#include <svc/FileSystem.hpp>

struct VFileMusic: public sf::Music
{
    VFile stream;
};

#endif //VFILE_MUSIC_HPP_INCLUDED
