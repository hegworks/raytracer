#pragma once

#include <Quad.h>

class QuadLight
{
public:
	Quad m_quad;
	float3 m_color = float3(1.0f);
	float m_intensity = 1.0f;

	QuadLight()
	{
		m_quad = Quad(1.0f);
	}

	float GetArea() const
	{
		return sqrf(m_quad.size * 2);
	}

	float3 GetRandomPoint(const float r0, const float r1) const
	{
		const float size = m_quad.size;
		const float3 corner1 = TransformPosition(float3(-size, 0, -size), m_quad.T);
		const float3 corner2 = TransformPosition(float3(size, 0, -size), m_quad.T);
		const float3 corner3 = TransformPosition(float3(-size, 0, size), m_quad.T);
		return corner1 + r0 * (corner2 - corner1) + r1 * (corner3 - corner1);
	}

	float3 GetRandomPoint(uint& seed) const
	{
		return GetRandomPoint(RandomFloat(seed), RandomFloat(seed));
	}
};
