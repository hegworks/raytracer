#pragma once

struct ALIGNED(32) Material
{
	enum class Type : std::uint8_t
	{
		DIFFUSE,
		DIFFUSE_PT,
		REFLECTIVE,
		GLOSSY,
		REFRACTIVE
	};

	float3 m_albedo = float3(1.0f);
	float m_factor = 1.0f;
	Type m_type = Type::DIFFUSE;
	uint32_t dummy[3];
};
