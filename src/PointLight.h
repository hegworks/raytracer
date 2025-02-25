#pragma once

#include "Color.h"

class PointLight
{
public:
	float3 m_pos = float3(0);
	float3 m_color = float3(Color::WHITE);
	float m_intensity = 1.0f;

	PointLight() = default;
	PointLight(float3 pos, float3 color, float intensity)
	{
		m_pos = pos;
		m_color = color;
		m_intensity = intensity;
	}
};
