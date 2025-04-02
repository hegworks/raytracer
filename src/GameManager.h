#pragma once

namespace Tmpl8
{
class Renderer;
}

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

	void Init(Scene* scene, Renderer* renderer);
	void Tick(const float deltaTime);
	void OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord);
	void OnMouseDown(int button);
	void OnMouseUp(int button);

private:
	void LoadLevel(const int levelIdx);

	float2 m_windowCoordF = 0;
	int2 m_windowCoord = 0;
	float2 m_screenCoordF = 0;
	int2 m_screenCoord = 0;
	int2 m_mouseDownWindowPos = 0;
	float2 m_mouseDelta = 0;
	bool m_isMouseLeftBtnDown = false;
	bool m_isMouseRightBtnDown = false;

	int m_levelObjectInstIdx = -1;
	float m_deltaTime = 0;

	Scene* m_scene = nullptr;
	Renderer* m_renderer = nullptr;

};
