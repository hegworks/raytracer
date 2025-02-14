#pragma once

#include <Quad.h>

class QuadLight
{
public:
	Quad m_quad;
	float m_pdf = 1.0f;

	QuadLight(int idx)
	{
		m_quad = Quad(idx, 1.0f);
	}
};
