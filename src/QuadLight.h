#pragma once

#include <Quad.h>

class QuadLight
{
public:
	Quad m_quad;
	float3 m_color = float3(1.0f);
	float m_intensity = 1.0f;

	QuadLight(int idx)
	{
		m_quad = Quad(idx, 1.0f);
	}

	/// returns a uniform pdf
	float GetPDF() const
	{
		const float area = sqrf(m_quad.size * 2);
		return 1.0f / area;
	}

	float3 GetRandomPoint(const float r0, const float r1) const
	{
		const float size = m_quad.size;
		float3 corner1 = TransformPosition(float3(-size, 0, -size), m_quad.T);
		float3 corner2 = TransformPosition(float3(size, 0, -size), m_quad.T);
		float3 corner3 = TransformPosition(float3(-size, 0, size), m_quad.T);
		return corner1 + r0 * (corner2 - corner1) + r1 * (corner3 - corner1);
	}

	float3 GetRandomPoint(uint& seed) const
	{
		return GetRandomPoint(RandomFloat(seed), RandomFloat(seed));
	}
};
