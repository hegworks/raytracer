#pragma once

class Material
{
public:
	enum class Type : std::uint8_t
	{
		DIFFUSE,
		DIFFUSE_PT,
		REFLECTIVE,
		GLOSSY,
		REFRACTIVE
	};

	float3 m_albedo = float3(1.0f);
	float m_glossiness = 1.0f;
	Type m_type = Type::DIFFUSE;
};
