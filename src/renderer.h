#pragma once
#include "RNG.h"

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace(Ray& ray, int pixelIndex, bool isTddX);
	void Tick(float deltaTime);
	void UI();
	void Shutdown() { /* implement if you want to do things on shutdown */ }
	// input handling
	void MouseUp(int button) { /* implement if you want to detect mouse button presses */ }
	void MouseDown(int button) { /* implement if you want to detect mouse button presses */ }
	void MouseMove(int x, int y) { mousePos.x = x, mousePos.y = y; }
	void MouseWheel(float y) { /* implement if you want to handle the mouse wheel */ }
	void KeyUp(int key) { /* implement if you want to handle keys */ }
	void KeyDown(int key) { /* implement if you want to handle keys */ }
	float3 CalcPointLight(float3 p, float3 n, float3 brdf, bool isTddPoint, bool isTddX);
	float3 CalcSpotLight(float3 p, float3 n, float3 brdf);
	float3 CalcDirLight(float3 p, float3 n, float3 brdf);
	float3 CalcQuadLight(float3 p, float3 n, float3 brdf, uint pixelIndex);
	float3 CalcLights(Ray& ray, uint pixelIndex, bool isTddX);

	// data members
	int2 mousePos;
	float4* accumulator;
	Scene scene;
	Camera camera;
	bool animating = false;
	float anim_time = 0;
	int ndal = 3; // normal, distance, albedo, light
	uint pixelSeeds[SCRWIDTH * SCRHEIGHT];
	inline static thread_local RNG threadRng;
	int qlNumSamples = 1;
	bool qlOneSided = true;
	int acmMax = 1;
	inline static int acmCounter;
	bool useACMMax = true;
};

} // namespace Tmpl8