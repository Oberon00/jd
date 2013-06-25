// Part of the Jade Engine -- Copyright (c) Christian Neumüller 2012--2013
// This file is subject to the terms of the BSD 2-Clause License.
// See LICENSE.txt or http://opensource.org/licenses/BSD-2-Clause

#ifndef SOUNDMANAGER_HPP_INCLUDED
#define SOUNDMANAGER_HPP_INCLUDED SOUNDMANAGER_HPP_INCLUDED

#include "compsys/Component.hpp"
#include "ressys/AutoResource.hpp"
#include "ressys/AutoSoundBuffer.hpp"
#include "ressys/VFileMusic.hpp"

#include <boost/ptr_container/ptr_vector.hpp>

#include <memory>
#include <string>


typedef AutoResource<sf::Sound, sf::SoundBuffer> AutoSound;

class SoundManager: public Component {
    JD_COMPONENT

public:
    SoundManager();
    ~SoundManager();

    void playSound(std::string const& name);
    void playSound(ResourceTraits<sf::SoundBuffer>::Ptr const& buf);
    void playSound(AutoSound& sound);

    void setBackgroundMusic(std::string const& name, sf::Time fadeDuration = sf::Time::Zero);
    void setBackgroundMusic(VFileMusic& music, sf::Time fadeDuration = sf::Time::Zero);

    void fade();
    void tidy();

private:
    struct FadedMusic {
        FadedMusic(): target(0), increment(0) { }
        FadedMusic(VFileMusic& music, float target, sf::Time fadeDuration);

        FadedMusic& operator= (FadedMusic&& rhs);
        bool fade();

        float target;
        std::unique_ptr<VFileMusic> music;
        float increment;
    };

    FadedMusic m_previousMusic;
    FadedMusic m_currentMusic;
    boost::ptr_vector<AutoSound> m_playingSounds;
};

#endif
