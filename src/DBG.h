#pragma once

static bool tdd = true;
inline bool tddResetCam = false;

static bool DBGCanPrint(const float2 pos)
{
	return !(pos.x < 0 || pos.x >= SCRWIDTH || pos.y < 0 || pos.y >= SCRHEIGHT);
}

static bool IsCloseF(const float a, const float b)
{
	return abs(a - b) < 0.01;
}
