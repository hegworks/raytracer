#pragma once

#include <array>

#include "renderer.h"

class Scene;
constexpr int LEVEL_COUNT = 1;

class GameManager
{
public:
	enum class State : int8_t
	{
		START_MENU,
		GAMEPLAY,
	};

	int m_levelIdx = 0;
	State m_state;

	void Init(Scene* scene);
	void Tick(float deltaTime);

	std::array<quat, LEVEL_COUNT> m_winQuatList;

private:
	void LoadLevel(int levelIdx);

	Scene* m_scene = nullptr;

};
