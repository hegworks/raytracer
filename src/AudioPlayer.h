#pragma once

#include "Audio/Sound.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace Uknitty
{

enum class AudioType
{
	BGM,
	WIN,
};

class AudioPlayer
{
public:
	AudioPlayer() = default;
	~AudioPlayer();

	/// will create the first instance of a sound
	void CreateSound(Uknitty::AudioType audioType, const std::string& filePath, Audio::Sound::Type soundType);
	/// will play a non-playing instance of the sound
	void ReplayOverlapped(Uknitty::AudioType audioType);
	/// will play the first created instance of the sound
	void Play(AudioType audioType);
	/// will play the first created instance of the sound from the beginning
	void Replay(AudioType audioType);
	/// will stop/pause the first instance of the sound
	void Stop(AudioType audioType);
	/// will seek the first instances of the sound
	void Seek(AudioType audioType, uint64_t milliseconds);
	/// sets volume of all instances of the sound
	void SetVolume(AudioType audioType, float volume);
	/// sets pitch of all instances of the sound
	void SetPitch(AudioType audioType, float pitch);

private:
	struct SoundSettings
	{
		float volume = 1.0f;
		float pitch = 1.0f;
	};

	std::unordered_map<Uknitty::AudioType, std::vector<Audio::Sound*>> m_soundInstances;
	std::unordered_map<Uknitty::AudioType, SoundSettings*> m_settings;

	Audio::Sound* CreateCopy(Uknitty::AudioType audioType);
	void CheckExistance(AudioType audioType);
	void ApplySettings(Audio::Sound* sound, AudioType audioType);
};

} // namespace Uknitty
