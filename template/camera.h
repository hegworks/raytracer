#pragma once

#include "DBG.h"

namespace Tmpl8
{
class Camera
{
public:
	Camera()
	{
		// setup a basic view frustum
		Reset();
	}
	Ray GetPrimaryRay(const float x, const float y)
	{
		// calculate pixel position on virtual screen plane
		const float u = (float)x * (1.0f / SCRWIDTH);
		const float v = (float)y * (1.0f / SCRHEIGHT);
		const float3 P = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft);
		return Ray(camPos, P - camPos);
	}
	bool HandleInput(const float t)
	{
		if(!WindowHasFocus()) return false;
		if(tddResetCam) { Reset(); tddResetCam = false; return false; }
		float speed = 0.0025f * t;
		float3 ahead = normalize(camTarget - camPos);
		float3 tmpUp(0, 1, 0);
		float3 right = normalize(cross(tmpUp, ahead));
		float3 up = normalize(cross(ahead, right));
		bool changed = false;
		if(IsKeyDown(GLFW_KEY_A)) camPos -= speed * 2 * right, changed = true;
		if(IsKeyDown(GLFW_KEY_D)) camPos += speed * 2 * right, changed = true;
		if(IsKeyDown(GLFW_KEY_W)) camPos += speed * 2 * ahead, changed = true;
		if(IsKeyDown(GLFW_KEY_S)) camPos -= speed * 2 * ahead, changed = true;
		if(IsKeyDown(GLFW_KEY_R)) camPos += speed * 2 * up, changed = true;
		if(IsKeyDown(GLFW_KEY_F)) camPos -= speed * 2 * up, changed = true;
		if(changed)
		{
			camTarget = camPos + ahead;
			ahead = normalize(camTarget - camPos);
			right = normalize(cross(tmpUp, ahead));
			up = normalize(cross(ahead, right));
		}
		bool rotated = false;
		if(IsKeyDown(GLFW_KEY_DOWN) && !tdd) camTarget -= speed * up, rotated = true;
		if(IsKeyDown(GLFW_KEY_UP) && !tdd) camTarget += speed * up, rotated = true;
		if(IsKeyDown(GLFW_KEY_LEFT)) camTarget -= speed * right, rotated = true;
		if(IsKeyDown(GLFW_KEY_RIGHT)) camTarget += speed * right, rotated = true;
		if(rotated)
		{
			ahead = normalize(camTarget - camPos);
			right = normalize(cross(tmpUp, ahead));
			up = normalize(cross(ahead, right));
			changed = true;
		}
		if(!changed) return false;
		topLeft = camPos + ahead * 2.0f - aspect * right + up;
		topRight = camPos + ahead * 2.0f + aspect * right + up;
		bottomLeft = camPos + ahead * 2.0f - aspect * right - up;
		return true;
	}
	void Reset()
	{
		camPos = {-4.03572845f, 4.74554300f, -4.8441176f};
		camTarget = {-3.48691559f, 4.21333361f, -4.19948006f};
		topLeft = {-3.94675612f,4.52773714f,-1.99716520f};
		topRight = {-1.23944438f, 4.52773714f, -4.30203772f};
		bottomLeft = {-4.63676071f, 2.83451128f, -2.80764723f};
	}
	float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camTarget;
	float3 topLeft, topRight, bottomLeft;
};
}