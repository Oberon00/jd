#include "SoundManager.hpp"
#include "Timer.hpp"
#include "ServiceLocator.hpp"

#include "ResourceManager.hpp"

#include <boost/bind.hpp>


SoundManager::SoundManager()
{
}

SoundManager::~SoundManager()
{
}

SoundManager::FadedMusic::FadedMusic(
	VFileMusic& music, float target, sf::Time fadeDuration):
	target(target),
	music(&music),
	increment(0)
{
    if (fadeDuration.asMicroseconds()) {
        music.setVolume(100 - target);
        increment = (target - music.getVolume()) / fadeDuration.asSeconds();
    } else {
        music.setVolume(target);
    }
}


SoundManager::FadedMusic& SoundManager::FadedMusic::operator= (FadedMusic&& rhs)
{
    target = rhs.target;
    increment = rhs.increment;
    music = std::move(rhs.music);
    return *this;
}

bool SoundManager::FadedMusic::fade()
{
    float frameSeconds = ServiceLocator::timer().frameDuration().asSeconds();
    float const newVolume = music->getVolume() + increment * frameSeconds;
    music->setVolume(newVolume);
    if (increment < 0 ? newVolume <= target : newVolume >= target) {
        music->setVolume(target);
        return true;
    }
    return false;
}

void SoundManager::playSound(std::string const& name)
{
    playSound(resMng<sf::SoundBuffer>().keepLoaded(name));
}

void SoundManager::playSound(ResourceTraits<sf::SoundBuffer>::Ptr const& buf)
{
    std::unique_ptr<AutoSound> snd(new AutoSound);
    snd->setResource(buf);
    snd->play();
    playSound(*snd.release());
}

void SoundManager::playSound(AutoSound& sound)
{
    m_playingSounds.push_back(&sound);
}

void  SoundManager::setBackgroundMusic(std::string const& name, sf::Time fadeDuration)
{
    std::unique_ptr<VFileMusic> music(new VFileMusic);
    music->stream.open(name);
    music->openFromStream(music->stream);
    music->play();
    setBackgroundMusic(*music.release(), fadeDuration);
}

void SoundManager::setBackgroundMusic(VFileMusic& music, sf::Time fadeDuration)
{
    if (m_currentMusic.music)
        m_previousMusic = FadedMusic(*m_currentMusic.music.release(), 0, fadeDuration);
    m_currentMusic = FadedMusic(music, 100, fadeDuration);
}

void SoundManager::tidy()
{
    m_playingSounds.erase(
	std::remove_if(m_playingSounds.begin(), m_playingSounds.end(),
            [](AutoSound& snd) { return snd.getStatus() != sf::Sound::Playing; }),
    m_playingSounds.end());
}

void SoundManager::fade()
{
    if (
        !m_currentMusic.music ||
        m_currentMusic.target == m_currentMusic.music->getVolume()
    ) {
        return;
    }
    if (m_previousMusic.music)
        m_previousMusic.fade();
    if (m_currentMusic.fade())
        m_previousMusic.music.reset();
}
