#pragma once

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
	void UI();
	void Shutdown() { /* implement if you want to do things on shutdown */ }
	// input handling
	void MouseUp([[maybe_unused]] int button) { /* implement if you want to detect mouse button presses */ }
	void MouseDown(int button);
	void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
	void MouseWheel([[maybe_unused]] float y) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp([[maybe_unused]] int key) { /* implement if you want to handle keys */ }
	void KeyDown(int key);
	void RotateAroundWorldAxis(Transform& transform, const float3& worldAxis, float angleRadians);

#ifdef SCALAR
	float3 CalcAllPointLightsScalar(float3 p, float3 n, float3 brdf);
	float3 CalcStochPointLightsScalar(float3 p, float3 n, float3 brdf, int pixelIndex);
#elif defined(DOD)
	float3 CalcAllPointLightsDOD(float3 p, float3 n, float3 brdf);
	float3 CalcStochPointLightsDOD(float3 p, float3 n, float3 brdf, int pixelIndex);
#elif defined(SIMD)
	float3 CalcAllPointLightsSIMD(float3 p, float3 n, float3 brdf);
#endif

	float3 CalcAllSpotLights(float3 p, float3 n, float3 brdf);
	float3 CalcSpotLight(const SpotLight& light, float3 p, float3 n, float3 brdf);
	float3 CalcAllDirLights(float3 p, float3 n, float3 brdf);
	float3 CalclDirLight(const DirLight& light, float3 p, float3 n, float3 brdf);
	float3 CalcAllQuadLights(float3 p, float3 n, float3 brdf, uint pixelIndex);
	float3 CalcQuadLight(const QuadLight& light, float3 p, float3 n, float3 brdf, uint pixelIndex);
	float3 CalcLights(Ray& ray, float3 p, float3 n, float3 brdf, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, bool isTddCameraY);

	// data members
	int2 mousePos;
	float4* accumulator;
	float4* illuminations;
	Camera camera;
	bool animating = false;
	float anim_time = 0;
	int ndal = 3; // normal, distance, albedo, light
	uint pixelSeeds[SCRWIDTH * SCRHEIGHT];
	uint lastPixelSeeds[SCRWIDTH * SCRHEIGHT];
	inline static thread_local RNG threadRng;
	int qlNumSamples = 1;
	bool qlOneSided = true;
	inline static int acmCounter;
	bool useACM = false;
	int maxDepth = 50;
	Scene scene;

	union quadf { __m128 f4; float f[4]; };
	union quadi { __m128i i4; int i[4]; };

	float hsum_ps_sse1(__m128 v)
	{
		__m128 shuf = _mm_shuffle_ps(v, v, _MM_SHUFFLE(2, 3, 0, 1));
		__m128 sums = _mm_add_ps(v, shuf);
		shuf = _mm_movehl_ps(shuf, sums);
		sums = _mm_add_ss(sums, shuf);
		return    _mm_cvtss_f32(sums);
	}

	float hsum_ps_sse3(__m128 v)
	{
		__m128 shuf = _mm_movehdup_ps(v);
		__m128 sums = _mm_add_ps(v, shuf);
		shuf = _mm_movehl_ps(shuf, sums);
		sums = _mm_add_ss(sums, shuf);
		return        _mm_cvtss_f32(sums);
	}
};

} // namespace Tmpl8
