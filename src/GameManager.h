#pragma once

#ifdef _GAME

#include "RNG.h"

class AudioManager;
enum class ModelType : uint8_t;
class CountdownTimer;

namespace Tmpl8
{
class Renderer;
}

class Scene;

class GameManager
{

public:
	enum class State : int8_t
	{
		START_MENU,
		GAMEPLAY,
		WIN,
		TUTORIAL,
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

	ModelType m_levelObjectModelType = {};
	int m_levelIdx = 0;
	State m_state;
	int m_tutorialStage = 0;
	bool m_showTutorial = true;
	bool m_showProgressBar = true;
	float m_progress = 0;

	static constexpr int NUM_LEVELS = 10;

	void LoadStartMenu() const;
	void Init(Scene* scene, Renderer* renderer);
	void Tick(const float deltaTime);
	float CalcProgress() const;
	float CalcProgressByFixedRot(const float3& targetRotDeg, const float3& weight) const;
	float CalcProgressByAnyRot() const;
	float CalcProgressByQuat(quat a, quat b);
	void OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord);
	void OnTransformChanged(int instanceIdx) const;
	void OnMouseDown(int button);
	void OnMouseUp(int button);
	void OnKeyDown(int key) const;
	void RotateRandomly();
	void RotateUntilLeastDiff(float leastDiff);
	void UpdateProgressBar(float progress);
	void ResetSceneLists();
	void ResetGameplayStates();
	void LoadLevel(const int levelIdx);

private:
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
	float m_levelObjectScale = 0;
	float m_winTimeProgress = 0;
	quat m_winQuat;

	bool m_isGameWon = false;
	bool m_isWinSlerpFinished = false;
	bool m_isShrinkDeformedFinished = false;
	bool m_isGrowFullFinished = false;

	WinType m_winType = WinType::ANY_ROT;
	AnyRotWinData m_anyRotWinData;
	SingleSidedWinData m_singleSidedWinData;
	DoubleSidedWinData m_doubleSidedWinData;

	static constexpr float DRAG_ROTATE_SPEED = 0.0005f;
	static constexpr float WIN_SLERP_SPEED = 0.01f;
	static constexpr float WIN_PERCENTAGE = 0.95f;
	static constexpr float WIN_SLERP_END_PROGRESS = 1.0f - 1e-6f;
	static constexpr float SCALE_TIME = 2000.0f;
	static constexpr float SCALE_FACTOR = 1.8f;

	Scene* m_scene = nullptr;
	Renderer* m_renderer = nullptr;
	RNG m_rng;
	CountdownTimer* m_scaleTimer = nullptr;
	AudioManager* m_audioManager = nullptr;
};

#endif
