#pragma once

#include "Color.h"

class ALIGNED(32) PointLight
{
public:
	float3 m_pos = float3(0);
	float3 m_color = float3(Color::WHITE);
	float m_intensity = 1.0f;
	float dummy;

	PointLight() = default;
	PointLight(const float3 pos, const float3 color, const float intensity)
	{
		m_pos = pos;
		m_color = color;
		m_intensity = intensity;
	}
};
