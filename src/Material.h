#pragma once

inline const char* MATERIAL_STRING[] =
{
	"DIFFUSE",
	"GLOSSY",
	"REFRACTIVE",
	"PATH_TRACED",
	"EMISSIVE",
};

struct ALIGNED(64) Material
{
	enum class Type : std::uint32_t
	{
		DIFFUSE,
		GLOSSY,
		REFRACTIVE,
		PATH_TRACED,
		EMISSIVE,
	};

	char m_name[40];
	float3 m_albedo = float3(1.0f);
	float m_factor0 = 0.0f;
	float m_factor1 = 0.0f;
	Type m_type = Type::PATH_TRACED;
};
