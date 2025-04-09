#include "precomp.h"

#include "AudioManager.h"

#include "AudioPlayer.h"
#include <Audio/Sound.hpp>

AudioManager::AudioManager()
{
	m_audioPlayer = new Uknitty::AudioPlayer();

	m_audioPlayer->CreateSound(Uknitty::AudioType::BGM, ASSETDIR + "Audio/BGM.mp3", Audio::Sound::Type::Music);
	m_audioPlayer->CreateSound(Uknitty::AudioType::WIN0, ASSETDIR + "Audio/Win0.mp3", Audio::Sound::Type::Sound);
	m_audioPlayer->CreateSound(Uknitty::AudioType::WIN1, ASSETDIR + "Audio/Win1.mp3", Audio::Sound::Type::Sound);
	m_audioPlayer->CreateSound(Uknitty::AudioType::WIN2, ASSETDIR + "Audio/Win2.mp3", Audio::Sound::Type::Sound);

	m_audioPlayer->SetVolume(Uknitty::AudioType::BGM, 0.55f);
}

void AudioManager::OnWin()
{
	m_lastPlayedWinSoundIdx = (m_lastPlayedWinSoundIdx + 1) % NUM_WIN_SOUNDS;
	m_audioPlayer->Replay(static_cast<Uknitty::AudioType>(m_lastPlayedWinSoundIdx));
}

void AudioManager::OnStartGame() const
{
	m_audioPlayer->Play(Uknitty::AudioType::BGM);
}
