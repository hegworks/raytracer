#pragma once

class ALIGNED(64) Transform
{
public:
	float3 m_pos = 0;
	quat m_rot = quat::identity();
	float3 m_scl = 1;
	float3 m_rotAngles = 0;
	float3 dummy;
	mat4 m_invT; // inverse transposed of the transform matrix (to multiply normals with)
};
