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

	void OnWin() const;
	void OnStartGame() const;

private:
	Uknitty::AudioPlayer* m_audioPlayer = nullptr;

};
