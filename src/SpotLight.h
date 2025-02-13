#pragma once

#include <common.h>

class SpotLight
{
public:
	float3 m_pos = float3(0);
	float3 m_color = float3(Color::WHITE);
	float m_intensity = 1.0f;

	float3 m_dir = float3(0, -1, 0);
	float m_cosI = 0.8f; /// cosine of the inner angle
	float m_cosO = 0.7f; /// cosine of the outer angle

	SpotLight() = default;
};
