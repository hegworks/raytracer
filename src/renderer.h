#pragma once
#include "RNG.h"

namespace Tmpl8
{

class Renderer : public TheApp
{
public:
	// game flow methods
	void Init();
	float3 Trace(Ray& ray, int x, int y);
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
	int2 WTS(float3 p); /// WorldToScreen;
	bool CanTDD(int objIdx, float3 p);
	// data members
	int2 mousePos;
	float4* accumulator;
	Scene scene;
	Camera camera;
	bool animating = false;
	float anim_time = 0;
	int nda = 3; // normal, distance, albedo, light
	uint pixelSeeds[SCRWIDTH * SCRHEIGHT];
	inline static thread_local RNG threadRng;
	int qlNumSamples = 1;
	bool qlOneSided = true;
	int acmMax = 1;
	inline static int acmCounter;
	bool useACMMax = true;
	float davg, dfps, drps; /// DEBUG
	bool useAA = false; /// Anti-Aliasing
	float tddSceneScale = 3;
	int2 tddOffset = int2(0, -110);
	float tddy = 0.25f;
	int tddrx = 20;
};

} // namespace Tmpl8