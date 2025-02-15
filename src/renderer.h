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
	// data members
	int2 mousePos;
	float4* accumulator;
	Scene scene;
	Camera camera;
	bool animating = true;
	float anim_time = 0;
	int nda = 3; // normal, distance, albedo, light
	uint pixelSeeds[SCRWIDTH * SCRHEIGHT];
	static thread_local RNG threadRng;
	int qlNumSamples = 1;
	bool qlOneSided = true;
	int acmMax = 500;
	inline static int acmCounter;
	bool uiChanged = false;
	bool useACMMax = true;
	float davg, dfps, drps; // DEBUG
};

} // namespace Tmpl8