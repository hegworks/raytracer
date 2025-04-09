#include "precomp.h"

#include "AudioPlayer.h"

#include <Audio/Sound.hpp>
#include <cstdint>
#include <stdexcept>
#include <string>

namespace Uknitty
{

AudioPlayer::~AudioPlayer()
{
	for(auto& [audioType, soundInstances] : m_soundInstances)
	{
		for(const auto& sound : soundInstances)
		{
			delete sound;
		}
		soundInstances.clear();
	}
	m_soundInstances.clear();
}

void AudioPlayer::ReplayOverlapped(const Uknitty::AudioType audioType)
{
	CheckExistance(audioType);
	for(const auto& sound : m_soundInstances[audioType])
	{
		if(!sound->isPlaying())
		{
			ApplySettings(sound, audioType);
			sound->replay();
			return;
		}
	}
	Audio::Sound* copy = CreateCopy(audioType);
	ApplySettings(copy, audioType);
	copy->replay();
}

void AudioPlayer::Play(const AudioType audioType)
{
	CheckExistance(audioType);
	ApplySettings(m_soundInstances[audioType][0], audioType);
	m_soundInstances[audioType][0]->play();
}

void AudioPlayer::Replay(const AudioType audioType)
{
	CheckExistance(audioType);
	ApplySettings(m_soundInstances[audioType][0], audioType);
	m_soundInstances[audioType][0]->replay();
}

void AudioPlayer::Stop(const AudioType audioType)
{
	CheckExistance(audioType);
	m_soundInstances[audioType][0]->stop();
}

void AudioPlayer::Seek(const AudioType audioType, const uint64_t milliseconds)
{
	CheckExistance(audioType);
	m_soundInstances[audioType][0]->seek(milliseconds);
}

void AudioPlayer::SetVolume(const AudioType audioType, const float volume)
{
	CheckExistance(audioType);
	m_settings[audioType]->volume = volume;

	for(auto& sound : m_soundInstances[audioType])
	{
		if(sound->isPlaying())
		{
			ApplySettings(sound, audioType);
		}
	}
}

void AudioPlayer::SetPitch(const AudioType audioType, const float pitch)
{
	CheckExistance(audioType);
	m_settings[audioType]->pitch = pitch;

	for(auto& sound : m_soundInstances[audioType])
	{
		if(sound->isPlaying())
		{
			ApplySettings(sound, audioType);
		}
	}
}

void AudioPlayer::CreateSound(const Uknitty::AudioType audioType, const std::string& filePath, const Audio::Sound::Type soundType)
{
	Audio::Sound* sound = new Audio::Sound(filePath, soundType);
	if(soundType != Audio::Sound::Type::Sound)
	{
		sound->setLooping(true);
	}
	m_soundInstances[audioType].push_back(sound);
	m_settings[audioType] = new SoundSettings();
}

Audio::Sound* AudioPlayer::CreateCopy(const Uknitty::AudioType audioType)
{
	const Audio::Sound* originalInstance = m_soundInstances[audioType][0];
	Audio::Sound* copyInstance = new Audio::Sound(*originalInstance);
	m_soundInstances[audioType].push_back(copyInstance);
	return copyInstance;
}

void AudioPlayer::CheckExistance(const AudioType audioType)
{
	if(m_soundInstances.find(audioType) == m_soundInstances.end())
	{
		throw std::runtime_error("no sound instance of audioType has been created");
	}
}

void AudioPlayer::ApplySettings(Audio::Sound* sound, const AudioType audioType)
{
	sound->setVolume(m_settings[audioType]->volume);
	sound->setPitch(m_settings[audioType]->pitch);
}

} // namespace Uknitty
