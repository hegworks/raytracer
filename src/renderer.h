#pragma once

#include "GameManager.h"
#include "RNG.h"
#include "Scene.h"

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace(Ray& ray, int pixelIndex, int depth, bool tddIsPixelX, bool tddIsPixelY);
	void Tick(float deltaTime);
	bool HandleKeyboardRotations(float deltaTime);
	void UI();
#ifdef _GAME
	void GameUI();
#endif
	void Shutdown() { /* implement if you want to do things on shutdown */ }
	// input handling
	void MouseUp(int button);
	void MouseDown(int button);
	void MouseMove(int x, int y);
	void MouseWheel([[maybe_unused]] float y) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp([[maybe_unused]] int key) { /* implement if you want to handle keys */ }
	void KeyDown(int key);
	static void RotateAroundWorldAxis(Transform& transform, const float3& worldAxis, float angleRadians);

#ifdef SCALAR
	float3 CalcAllPointLightsScalar(const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcStochPointLightsScalar(const float3& p, const float3& n, const float3& brdf, int pixelIndex);
#elif defined(DOD)
	float3 CalcAllPointLightsDOD(const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcStochPointLightsDOD(const float3& p, const float3& n, const float3& brdf, int pixelIndex);
#elif defined(SIMD)
	float3 CalcAllPointLightsSIMD(const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcStochPointLightsSIMD(const float3& p, const float3& n, const float3& brdf, int pixelIndex);
#elif defined(AVX)
	float3 CalcAllPointLightsAVX(const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcStochPointLightsAVX(const float3& p, const float3& n, const float3& brdf, int pixelIndex);
#endif

	float3 CalcAllSpotLights(const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcSpotLight(const SpotLight& light, const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcAllDirLights(const float3& p, const float3& n, const float3& brdf) const;
	float3 CalclDirLight(const DirLight& light, const float3& p, const float3& n, const float3& brdf) const;
	float3 CalcAllQuadLights(const float3& p, const float3& n, const float3& brdf, uint pixelIndex);
	float3 CalcQuadLight(const QuadLight& light, const float3& p, const float3& n, const float3& brdf, uint pixelIndex);
	float3 CalcLights(Ray& ray, float3 p, float3 n, float3 brdf, uint pixelIndex);

	// data members
	ImFont* font = nullptr;

	int2 mousePos;
	float2 windowCoordF;
	int2 windowCoord;
	float2 screenCoordF;
	int2 screenCoord;
	float4* accumulator;
	float4* illuminations;
	int hoveredInst = -1;
	int hoveredMesh = -1;
	Camera camera;
	bool resetAccumulator = false;
	float anim_time = 0;
	int ndal = 3; // normal, distance, albedo, light
	uint pixelSeeds[SCRSIZE];
	uint lastPixelSeeds[SCRSIZE];
	inline static thread_local RNG threadRng;
	int qlNumSamples = 1;
	bool qlOneSided = true;
	inline static int acmCounter;
	bool useACM = false;
	Scene scene;

#ifdef _GAME
	GameManager m_gameManager;
#endif

	union quadf { __m128 f4; float f[4]; };
	union quadi { __m128i i4; int i[4]; };
	union octf { __m256 f8; float f[8]; };
	union octi { __m256 i8; float i[8]; };
};

} // namespace Tmpl8
