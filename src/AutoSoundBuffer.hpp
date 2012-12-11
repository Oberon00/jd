#ifndef AUTO_SOUND_BUFFER_HPP_INCLUDED
#define AUTO_SOUND_BUFFER_HPP_INCLUDED AUTO_SOUND_BUFFER_HPP_INCLUDED

#include <SFML/Audio/Sound.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include "resfwd.hpp"

inline void updateAutoResourceMedia(sf::Sound& s, ResourceTraits<sf::SoundBuffer>::Ptr buf)
{
    s.setBuffer(*buf);
}

#endif //AUTO_SOUND_BUFFER_HPP_INCLUDED