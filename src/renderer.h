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
#ifndef NPLS
	void CalcStochPointLights(float3 p, float3 n, float3 brdf, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, int numSamples, float3& stochasticL, uint numLights);
#else
	void CalcStochPointLightsSIMD(float3 p, float3 n, float3 brdf, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, int numSamples, float3& stochasticL, uint numLights, bool isAll);
#endif
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
	float3 CalcAllPointLights(float3 p, float3 n, float3 brdf, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, bool isTddCameraY);
	float3 CalcPointLight(const PointLight& light, float3 p, float3 n, float3 brdf, bool isTddPixelX, bool isTddPixelY);
	float3 CalcPointLightSIMD();
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
};

} // namespace Tmpl8
