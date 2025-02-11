#pragma once

#include "tmpl8math.h"

class Color
{
private:
	inline static constexpr float ZERO = 0.001f;
public:
	inline static const float3 WHITE = float3(1.0, 1.0, 1.0);
	inline static const float3 RED = float3(1.0, ZERO, ZERO);
	inline static const float3 GREEN = float3(ZERO, 1.0, ZERO);
	inline static const float3 BLUE = float3(ZERO, ZERO, 1.0);
	inline static const float3 MAGENTA = float3(1.0, ZERO, 1.0);
};
