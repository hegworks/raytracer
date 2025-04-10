#pragma once

#include "Material.h"

// -----------------------------------------------------------
// Quad primitive
// Oriented quad, intended to be used as a light source.
// -----------------------------------------------------------
class Quad
{
public:
	float size = 0;
	mat4 T, invT;
	Material m_material;
	float3 m_pos = float3(0);
	float3 m_dir = float3(0, 1, 0);

	Quad() = default;
	Quad(const float s, const mat4& transform = mat4::Identity())
	{
		size = s * 0.5f;
		T = transform, invT = transform.FastInvertedTransformNoScale();
	}
	void Intersect(Ray& ray) const
	{
		const float Oy = invT.cell[4] * ray.O.x + invT.cell[5] * ray.O.y + invT.cell[6] * ray.O.z + invT.cell[7];
		const float Dy = invT.cell[4] * ray.D.x + invT.cell[5] * ray.D.y + invT.cell[6] * ray.D.z;
		const float t = Oy / -Dy;
		if(t < ray.hit.t && t > 0)
		{
			const float Ox = invT.cell[0] * ray.O.x + invT.cell[1] * ray.O.y + invT.cell[2] * ray.O.z + invT.cell[3];
			const float Oz = invT.cell[8] * ray.O.x + invT.cell[9] * ray.O.y + invT.cell[10] * ray.O.z + invT.cell[11];
			const float Dx = invT.cell[0] * ray.D.x + invT.cell[1] * ray.D.y + invT.cell[2] * ray.D.z;
			const float Dz = invT.cell[8] * ray.D.x + invT.cell[9] * ray.D.y + invT.cell[10] * ray.D.z;
			const float Ix = Ox + t * Dx, Iz = Oz + t * Dz;
			if(Ix > -size && Ix < size && Iz > -size && Iz < size)
			{
				ray.hit.t = t;
				//ray.hit.prim = -1;
			}
		}
	}
	bool IsOccluded(const Ray& ray) const
	{
		const float Oy = invT.cell[4] * ray.O.x + invT.cell[5] * ray.O.y + invT.cell[6] * ray.O.z + invT.cell[7];
		const float Dy = invT.cell[4] * ray.D.x + invT.cell[5] * ray.D.y + invT.cell[6] * ray.D.z;
		const float t = Oy / -Dy;
		if(t < ray.hit.t && t > 0)
		{
			const float Ox = invT.cell[0] * ray.O.x + invT.cell[1] * ray.O.y + invT.cell[2] * ray.O.z + invT.cell[3];
			const float Oz = invT.cell[8] * ray.O.x + invT.cell[9] * ray.O.y + invT.cell[10] * ray.O.z + invT.cell[11];
			const float Dx = invT.cell[0] * ray.D.x + invT.cell[1] * ray.D.y + invT.cell[2] * ray.D.z;
			const float Dz = invT.cell[8] * ray.D.x + invT.cell[9] * ray.D.y + invT.cell[10] * ray.D.z;
			const float Ix = Ox + t * Dx, Iz = Oz + t * Dz;
			return Ix > -size && Ix < size && Iz > -size && Iz < size;
		}
		return false;
	}
	float3 GetNormal() const
	{
		return float3(-T.cell[1], -T.cell[5], -T.cell[9]);
	}
	/*float3& GetAlbedo(const float3 I)
	{
		return m_material.m_albedo;
	}*/
};
