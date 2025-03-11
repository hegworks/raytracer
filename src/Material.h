#pragma once

inline const char* MATERIAL_STRING[] =
{
	"DIFFUSE",
	"DIFFUSE_PT",
	"REFLECTIVE",
	"GLOSSY",
	"REFRACTIVE"
};

struct ALIGNED(64) Material
{
	enum class Type : std::uint32_t
	{
		DIFFUSE,
		DIFFUSE_PT,
		REFLECTIVE,
		GLOSSY,
		REFRACTIVE
	};

	char m_name[40];
	float3 m_albedo = float3(1.0f);
	float m_factor0 = 1.0f;
	float m_factor1 = 1.5f;
	Type m_type = Type::DIFFUSE_PT;
};
