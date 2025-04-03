#pragma once
#include "RNG.h"

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

	enum class WinType : int8_t
	{
		ANY_ROT,
		SINGLE_SIDED,
		DOUBLE_SIDED,
	};

	struct AnyRotWinData
	{
		quat m_winQuat = quat::identity();
	};

	struct SingleSidedWinData
	{
		float3 m_winRotDeg = 0;
		float3 m_winRotWeights = 0.33f;
	};

	struct DoubleSidedWinData
	{
		float3 m_winRotDeg0 = 0;
		float3 m_winRotDeg1 = 0;
		float3 m_winRotWeights0 = 0.33f;
		float3 m_winRotWeights1 = 0.33f;
	};

	int m_levelIdx = 0;
	State m_state;

	void Init(Scene* scene, Renderer* renderer);
	void Tick(const float deltaTime);
	float CalcProgress() const;
	float CalcProgressByFixedRot(const float3& targetRotDeg, const float3& weight) const;
	float CalcProgressByAnyRot() const;
	void OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord);
	void OnMouseDown(int button);
	void OnMouseUp(int button);
	void OnKeyDown(int key) const;
	void RotateRandomly();
	void RotateUntilLeastDiff(float leastDiff);
	void UpdateProgressBar(float progress) const;

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
	uint m_seed = 0;

	WinType m_winType = WinType::ANY_ROT;
	AnyRotWinData m_anyRotWinData;
	SingleSidedWinData m_singleSidedWinData;
	DoubleSidedWinData m_doubleSidedWinData;

	Scene* m_scene = nullptr;
	Renderer* m_renderer = nullptr;
	RNG m_rng;

};
