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
	void OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord);
	void OnMouseDown(int button);
	void OnMouseUp(int button);

private:
	void LoadLevel(int levelIdx);

	float2 m_windowCoordF = 0;
	int2 m_windowCoord = 0;
	float2 m_screenCoordF = 0;
	int2 m_screenCoord = 0;
	float2 m_mouseDownWindowPos = 0;

	Scene* m_scene = nullptr;

};
