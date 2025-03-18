#pragma once

inline const char* MATERIAL_STRING[] =
{
	"DIFFUSE",
	"DIFFUSE_PT",
	"GLOSSY",
	"GLOSSY_PT",
	"GLOSSY_PT2",
	"REFRACTIVE",
	"EMISSIVE",
	"PATH_TRACED",
};

struct ALIGNED(64) Material
{
	enum class Type : std::uint32_t
	{
		DIFFUSE,
		DIFFUSE_PT,
		GLOSSY,
		GLOSSY_PT,
		GLOSSY_PT2,
		REFRACTIVE,
		EMISSIVE,
		PATH_TRACED,
	};

	char m_name[40];
	float3 m_albedo = float3(1.0f);
	float m_factor0 = 0.5f;
	float m_factor1 = 0.5f;
	Type m_type = Type::PATH_TRACED;
};
