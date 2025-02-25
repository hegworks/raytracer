#pragma once

#include "Color.h"

class DirLight
{
public:
	float3 m_color = float3(Color::WHITE);
	float m_intensity = 1.0f;
	float3 m_dir = float3(0, -1, 0);
};
