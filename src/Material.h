﻿#pragma once

inline const char* MATERIAL_STRING[] =
{
	"DIFFUSE",
	"DIFFUSE_PT",
	"REFLECTIVE",
	"GLOSSY",
	"REFRACTIVE"
};

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

	char m_name[32];
	float3 m_albedo = float3(1.0f);
	float m_factor0 = 1.0f;
	float m_factor1 = 1.0f;
	Type m_type = Type::DIFFUSE_PT;
};
