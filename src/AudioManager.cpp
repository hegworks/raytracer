#include "precomp.h"

#include "AudioManager.h"

#include "AudioPlayer.h"
#include <Audio/Sound.hpp>

AudioManager::AudioManager()
{
	m_audioPlayer = new Uknitty::AudioPlayer();

	m_audioPlayer->CreateSound(Uknitty::AudioType::BGM, ASSETDIR + "Audio/BGM_Slow.ogg", Audio::Sound::Type::Music);
	m_audioPlayer->CreateSound(Uknitty::AudioType::WIN, ASSETDIR + "Audio/BGM_Fast.ogg", Audio::Sound::Type::Sound);

	//m_audioPlayer->SetVolume(Uknitty::AudioType::BGM, 0.5f);
	//m_audioPlayer->SetVolume(Uknitty::AudioType::WIN, 0.25f);

	//m_audioPlayer->Replay(Uknitty::AudioType::BGM);
}

void AudioManager::OnWin() const
{
	m_audioPlayer->Replay(Uknitty::AudioType::WIN);
}

void AudioManager::OnStartGame() const
{
	m_audioPlayer->Play(Uknitty::AudioType::BGM);
}

#if 0
void AudioManager::OnEnemyHurt()
{
	m_lastEnemyHurtAudioIndex = (m_lastEnemyHurtAudioIndex + 1) % ENEMY_HURT_TYPES.size();
	m_audioPlayer->ReplayOverlapped(ENEMY_HURT_TYPES[m_lastEnemyHurtAudioIndex]);
}

void AudioManager::OnPlayerShotGun()
{
	m_audioPlayer->ReplayOverlapped(Uknitty::AudioType::PlayerGunShot);
}

void AudioManager::OnEnemyShotGun()
{
	m_audioPlayer->ReplayOverlapped(Uknitty::AudioType::EnemyGunShot);
}

void AudioManager::OnLevelAlert()
{
	m_audioPlayer->SetPitch(Uknitty::AudioType::BGM_Slow, BGM_ALERT_PITCH);
}

void AudioManager::OnLevelNormal()
{
	m_audioPlayer->SetPitch(Uknitty::AudioType::BGM_Slow, 1.0f);
}

void AudioManager::OnPause()
{
	m_audioPlayer->Stop(Uknitty::AudioType::BGM_Slow);
}

void AudioManager::OnResume()
{
	m_audioPlayer->Play(Uknitty::AudioType::BGM_Slow);
}

void AudioManager::OnMainMenu()
{
	m_audioPlayer->Seek(Uknitty::AudioType::BGM_Slow, 0);
	m_audioPlayer->Stop(Uknitty::AudioType::BGM_Slow);
}
#endif
