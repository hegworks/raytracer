// Template, 2024 IGAD Edition
// Get the latest version from: https://github.com/jbikker/tmpl8
// IGAD/NHTV/BUAS/UU - Jacco Bikker - 2006-2024

// common.h is to be included in host and device code and stores
// global settings and defines.

#pragma once

// default screen resolution
#define SCRWIDTH	1280
#define SCRHEIGHT	720
// #define FULLSCREEN
// #define DOUBLESIZE

// constants
#define PI				3.14159265358979323846264f
#define INVPI			0.31830988618379067153777f
#define INV2PI			0.15915494309189533576888f
#define TWOPI			6.28318530717958647692528f
#define DEG_TO_RAD(x)	((x)*0.01745329251f)
#define RAD_TO_DEG(x)	((x)*57.2957795131f)
#define SQRT_PI_INV		0.56418958355f
#define LARGE_FLOAT		1e34f
//#define EPS				1e-4f
const std::string ASSETDIR("../assets/");
