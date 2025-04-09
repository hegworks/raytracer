#pragma once

namespace Uknitty
{
enum class AudioType;
class AudioPlayer;
}

class AudioManager
{
public:
	AudioManager();

	void OnWin();
	void OnStartGame() const;

private:
	Uknitty::AudioPlayer* m_audioPlayer = nullptr;

	int m_lastPlayedWinSoundIdx = -1;
	static constexpr int NUM_WIN_SOUNDS = 3;
};
