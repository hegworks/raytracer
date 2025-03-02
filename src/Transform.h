#pragma once

class Transform
{
public:
	float3 m_pos = 0;
	float3 m_rot = 0;
	float3 m_scl = 1;
	mat4 m_invT;
};
