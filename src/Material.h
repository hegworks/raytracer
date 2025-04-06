#pragma once

inline const char* MATERIAL_STRING[] =
{
	"DIFFUSE",
	"GLOSSY",
	"REFRACTIVE",
	"PATH_TRACED",
	"EMISSIVE",
};

constexpr float DEFAULT_ALBEDO = 0.7f;

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
	float3 m_albedo = float3(DEFAULT_ALBEDO);

	/// <summary>
	/// for DIFFUSE	-> UNUSED<br>
	/// for GLOSSY	-> UNUSED<br>
	/// for REFRACTIVE	-> density<br>
	/// for EMISSIVE	-> intensity<br>
	/// for PATH_TRACED	-> smoothness<br>
	/// </summary>
	float m_factor0 = 0.0f;

	/// /// <summary>
	/// for DIFFUSE	-> UNUSED<br>
	/// for GLOSSY	-> UNUSED<br>
	/// for REFRACTIVE	-> ior<br>
	/// for EMISSIVE	-> UNUSED<br>
	/// for PATH_TRACED	-> specularity<br>
	/// </summary>
	float m_factor1 = 0.0f;

	Type m_type = Type::PATH_TRACED;
};
