#pragma once

inline bool tdd = true;
inline bool tddResetCam = false;
inline float tddSceneScale = 3;
inline int2 tddOffset = int2(0, -110);
inline float tddy = 0.25f;
inline int tddrx = 20;
inline bool tddPRay = true; /// PrimaryRay
inline bool tddPRayL = true; /// PrimaryRayLength
inline bool tddPN = true; /// P Normal
inline bool tddPNL = true; /// P Normal Length

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
