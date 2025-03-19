#pragma once
#include "UknittyMath.h"

inline bool tdd = false;
inline bool tddResetCam = false;
inline float tddSceneScale = 2.0f;
inline int2 tddOffset = int2(0, 0);
inline float tddy = 0.25f;
inline int tddrx = 20;
inline bool tddSXM = false; /// SingleXMode
inline int tddSXX = 0; /// SingleXX
inline int tddSliceY = SCRHEIGHT / 2; /// SingleXX
inline int tddFS = 2; /// FontSize
inline bool tddRC = true; /// RayCoord

inline bool tddBBG = false; /// BlackBackGround
inline bool tddPRay = true; /// PrimaryRay
inline bool tddPRayL = false; /// PrimaryRayLength
inline bool tddPN = true; /// P Normal
inline bool tddPNL = false; /// P Normal Length

inline bool tddPLR = false; /// PointLightRay
inline bool tddPLP = false; /// PointLightPosition

inline float davg, dfps, drps; /// DEBUG
inline bool useAA = false; /// Anti-Aliasing

inline float dbgIor = 1.5f;
inline bool dbgBeer = true; /// beer's law

inline bool isDbgPixel = false;
inline bool isDbgPixelClicked = false;
inline bool isDbgPixelEntered = false;
inline int2 dbgpixel{0,0};
inline bool isDbgFixSeed = false;
inline int2 dbgScrRangeX = {0,SCRWIDTH};
inline int2 dbgScrRangeY = {0,SCRHEIGHT};

inline bool useSD = true; /// UseSkyDome
inline float dbgSDBF = 1.0f; /// SkyDomeBrightnessFactor
inline float EPS = 1e-3f;
inline int epsInt = 2;
inline float dbgFF = 10.f; /// FireFly
inline float dbgGC = 1.35f; /// GammaCorrection
inline bool dbgSL = true; /// StochasticLights
inline int dbgSLS = 2; /// StochasticLightsSamples
inline bool dbgSF = true; /// StochasticFresnel

inline bool useDOF = false; /// DepthOfField
inline float defocusAngle = 1;
inline float focusDistance = 5;
inline int spp = 1; /// SamplesPerPixels

inline int selectedIdx = 0;

inline int dbgRotAxisInt = 1; /// 0->X 1->Y 2->Z
inline float3 rotAxis(0, 1, 0);

static bool DBGCanPrint(const float2 pos)
{
	return !(pos.x < 0 || pos.x >= SCRWIDTH || pos.y < 0 || pos.y >= SCRHEIGHT - 10);
}

static bool IsCloseF(const float a, const float b)
{
	return abs(a - b) < 0.001;
}

/// World To Screen
static int2 WTS(float3 p)
{
	int x = static_cast<int>(floorf(range_to_range(-4.0f * tddSceneScale, 4.0f * tddSceneScale, 0, SCRWIDTH, p.x))) + tddOffset.x;
	int y = static_cast<int>(floorf(range_to_range(2.25f * tddSceneScale, -2.25f * tddSceneScale, 0, SCRHEIGHT, p.z))) + tddOffset.y;
	return {x,y};
}

/// 2D Debugger Primary/Point
static void TDDP(Ray& ray, float3 p, float3 n, Surface* screen, int depth, bool tddIsPixelX, bool tddIsPixelY, bool tddIsCameraY)
{
	int2 pd = WTS(p); /// intersection point debug
	if(tddIsCameraY)
	{
		screen->Plot(pd.x, pd.y, 0xffffff);
	}

	if(tddIsPixelX && tddIsPixelY)
	{
		// ray
		if(tddPRay)
		{
			float2 o = WTS(ray.O);
			float2 d = pd;
			uint color = 0xffffff;
			if(depth > 0)
			{
				int colordepth = (depth - 1) % 3;
				if(colordepth == 0) color = 0xff0000;
				if(colordepth == 1) color = 0x00ff00;
				if(colordepth == 2) color = 0x0000ff;
			}
			bool inside = dot(ray.D, n) > 0;
			if(inside)
			{
				color = 0xffff00;
			}
			screen->Line(o.x, o.y, d.x, d.y, color);
		}

		// ray coord
		if(tddRC)
		{
			float2 o = pd;
			o += 5;
			char t[50];
			sprintf(t, "%.2f,%.2f,%.2f", p.x, p.y, p.z);
			if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0x00ff00, tddFS);
		}

		// ray length
		if(tddPRayL)
		{
			int2 o = {pd.x, pd.y - 5};

			char t[20];
			sprintf(t, "%.2f", ray.hit.t);
			if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0xff0000, tddFS);
		}

		// normal
		if(tddPN)
		{
			float2 o = pd;
			float2 d = WTS(p + n * 0.2f);
			screen->Line(o.x, o.y, d.x, d.y, 0xff00ff);
		}

		// normal length
		if(tddPNL)
		{
			int2 o = WTS(p + n * 0.2f); /// normal debug point

			char t[20];
			sprintf(t, "%.2f", length(n));
			if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0xff00ff, tddFS);
		}
	}
}
