// Template, 2024 IGAD Edition
// Get the latest version from: https://github.com/jbikker/tmpl8
// IGAD/NHTV/BUAS/UU - Jacco Bikker - 2006-2024

// common.h is to be included in host and device code and stores
// global settings and defines.

#pragma once

// default screen resolution
#if defined(_DEBUG)
constexpr float SCRSCALE = 5.0f;
#else
constexpr float SCRSCALE = 1.75f;
#endif
constexpr float INV_SCRSCALE = 1.0f / SCRSCALE;
constexpr int WINDOWWIDTH = 1280;
constexpr int WINDOWHEIGHT = 720;
constexpr int SCRWIDTH = static_cast<int>(WINDOWWIDTH * INV_SCRSCALE);
constexpr int SCRHEIGHT = static_cast<int>(WINDOWHEIGHT * INV_SCRSCALE);
constexpr int SCRSIZE = static_cast<int>(SCRWIDTH * SCRHEIGHT);
// #define FULLSCREEN
 //#define DOUBLESIZE

// constants
#define LARGE_FLOAT		1e34f
//#define EPS				1e-4f
const std::string ASSETDIR("../assets/");


thread_local static int counter = 0;
thread_local static float summ = 0;
thread_local static bool resultPrinted = false;
constexpr int SAMPLES = 1000000;

struct ScopedTimer
{
	Timer t;
	const char* name;

	ScopedTimer(const char* funcName)
	{
		name = funcName;
	}

	~ScopedTimer()
	{
		if(counter++ < SAMPLES)
		{
			float seconds = t.elapsed();
			summ += seconds * 1000 * 1000;
		}
		else
		{
			if(resultPrinted) return;
			printf("Avg %s: %f Ms \n", name, summ / (float)SAMPLES);
			resultPrinted = true;
		}
	}
};
