#pragma once

#include "Color.h"

class ALIGNED(32) PointLight
{
public:
	float x, y, z;
	float r, g, b;
	//float3 m_color;
	float i;
	float dummy;

	PointLight() = default;
	PointLight(float3 pos, float3 color, float intensity)
	{
		x = pos.x;
		y = pos.y;
		z = pos.z;
		r = color.x;
		g = color.y;
		b = color.z;
		//m_color = color;
		i = intensity;
	}
};
