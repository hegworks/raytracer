#pragma once

class LaserLight
{
public:
	float3 m_pos = float3(0);
	float3 m_color = float3(Color::RED);
	float m_intensity = 10.0f;
	float3 m_dir = float3(0, -1, 0);
	float m_radius = 0.02f; // Radius of the laser beam cylinder
	float m_range = 100.0f; // Maximum range of the laser

	LaserLight() = default;
};
