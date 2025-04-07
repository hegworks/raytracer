#pragma once

#include "DBG.h"

namespace Tmpl8
{
class Camera
{
public:
	Camera()
	{
		camPos = {0.00000000f	,5.84618282f ,-9.78223801f};
		camTarget = {0.00000000f	,5.54261971f ,-8.82942677f};
		topLeft = {-1.77777779f	,6.19186783f ,-7.57305241f};
		topRight = {1.77777779f	,6.19186783f ,-7.57305241f};
		bottomLeft = {-1.77777779f	,4.28624535f ,-8.18017864f};
	}
	Ray GetPrimaryRay(const float x, const float y, const float2 randOnUnitDisk)
	{
		// pixel coord -> point on virutal screen plane
		const float u = (float)x * (1.0f / SCRWIDTH);
		const float v = (float)y * (1.0f / SCRHEIGHT);
		float3 vsp = topLeft + u * (topRight - topLeft) + v * (bottomLeft - topLeft); /// VirtualScreenPoint

		if(!useDOF) return Ray(camPos, vsp - camPos);

#pragma region DepthOfField
		// direction from camPos to vsp
		float3 vspDir = normalize(vsp - camPos);

		// Point on focal plane
		float3 fpp = camPos + vspDir * focusDistance; /// FocalPlanePoint

		// Calculate defocus radius
		float apertureRadius = focusDistance * tan(DEG_TO_RAD(defocusAngle / 2.0f));

		// Create orthonormal basis around the ray direction
		float3 w = vspDir;
		float3 apertureU = normalize(cross(w, float3(0, 1, 0)));
		float3 apertureV = cross(w, apertureU);

		float3 randOrigin = camPos + (randOnUnitDisk.x * apertureRadius * apertureU) + (randOnUnitDisk.y * apertureRadius * apertureV);

		return Ray(randOrigin, fpp - randOrigin);
#pragma endregion
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
		camPos = {0.00000000f,	0.206065923f,-9.24263668f};
		camTarget = {0.00000000f,	0.181073740f,-8.24294853f};
		topLeft = {-1.77777779f,	1.15576923f,-7.21826935f};
		topRight = {1.77777779f,	1.15576923f,-7.21826935f};
		bottomLeft = {-1.77777779f,	-0.843606114f,-7.26825333f};
	}
	float aspect = (float)SCRWIDTH / (float)SCRHEIGHT;
	float3 camPos, camTarget;
	float3 topLeft, topRight, bottomLeft;
};
}