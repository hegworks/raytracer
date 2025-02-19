#pragma once
#include "UknittyMath.h"

inline bool tdd = false;
inline bool tddResetCam = false;
inline float tddSceneScale = 3;
inline int2 tddOffset = int2(0, -110);
inline float tddy = 0.25f;
inline int tddrx = 20;

inline bool tddBBG = false; /// BlackBackGround
inline bool tddPRay = true; /// PrimaryRay
inline bool tddPRayL = true; /// PrimaryRayLength
inline bool tddPN = true; /// P Normal
inline bool tddPNL = true; /// P Normal Length

inline bool tddPLR = true; /// PointLightRay
inline bool tddPLP = true; /// PointLightPosition

inline float davg, dfps, drps; /// DEBUG
inline bool useAA = false; /// Anti-Aliasing

static bool DBGCanPrint(const float2 pos)
{
	return !(pos.x < 0 || pos.x >= SCRWIDTH || pos.y < 0 || pos.y >= SCRHEIGHT - 10);
}

static bool IsCloseF(const float a, const float b)
{
	return abs(a - b) < 0.001;
}

static bool IsTddPoint(int objIdx, float pY, float cameraY)
{
	return IsCloseF(pY, cameraY) && (objIdx == 1 || objIdx == 3 || objIdx == 10);
}

/// World To Screen
static int2 WTS(float3 p)
{
	int x = static_cast<int>(floorf(range_to_range(-4.0f * tddSceneScale, 4.0f * tddSceneScale, 0, SCRWIDTH, p.x))) + tddOffset.x;
	int y = static_cast<int>(floorf(range_to_range(2.25f * tddSceneScale, -2.25f * tddSceneScale, 0, SCRHEIGHT, p.z))) + tddOffset.y;
	return {x,y};
}

/// 2D Debugger Primary/Point
static void TDDP(Ray& ray, bool isTddX, float3 p, float3 n, Surface* screen)
{
	int2 pd = WTS(p); /// intersection point debug
	screen->Plot(pd.x, pd.y, 0xffffff);

	if(isTddX)
	{
		// primary ray
		if(tddPRay)
		{
			float2 o = WTS(ray.O);
			float2 d = pd;
			screen->Line(o.x, o.y, d.x, d.y, 0xff0000);
		}

		// normal
		if(tddPN)
		{
			float2 o = pd;
			float2 d = WTS(p + n / 2.0f);
			screen->Line(o.x, o.y, d.x, d.y, 0x00ff00);
		}

		// normal length
		if(tddPNL)
		{
			int2 o = WTS(p + n / 2.0f); /// normal debug point

			char t[20];
			sprintf(t, "%.2f", length(n));
			if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0x00ff00);
		}

		// ray length
		if(tddPRayL)
		{
			int2 o = {pd.x, pd.y - 5};

			char t[20];
			sprintf(t, "%.2f", ray.t);
			if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0xff0000);
		}
	}
}
