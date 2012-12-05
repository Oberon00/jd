#ifndef SOUNDMANAGER_HPP_INCLUDED
#define SOUNDMANAGER_HPP_INCLUDED SOUNDMANAGER_HPP_INCLUDED

#include <memory>
#include <boost/ptr_container/ptr_vector.hpp>
#include "AutoResource.hpp"
#include "AutoSoundBuffer.hpp"
#include "VFileMusic.hpp"
#include "compsys/Component.hpp"
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
	// struct SoundLengthLess: std::binary_function<sf::Sound const&, sf::Sound const&, bool> {
		// bool operator() (sf::Sound const& lhs, sf::Sound const& rhs) const
		// {
			// return remainingLength(lhs) < remainingLenght(rhs);
		// }
	// private:
		// sf::Time remainingLenght(sf::Sound const& sound) const
		// {
			// return sound.getBuffer()->getDuration() - sound.getPlayingOffset();
		// }
	// };
	
	FadedMusic m_previousMusic;
	FadedMusic m_currentMusic;
	boost::ptr_vector<AutoSound> m_playingSounds;
};

#endif

