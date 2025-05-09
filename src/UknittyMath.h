#pragma once

enum class Axis : uint8_t { X, Y, Z };

/// <summary>
/// map the number n from the range [x, y] to [a, b]
/// </summary>
inline float range_to_range(const float x, const float y, const float a, const float b, const float n)
{
	return (((n - x) / (y - x)) * (b - a)) + a;
}

inline float hsum_ps_sse3(const __m128 v)
{
	__m128 shuf = _mm_movehdup_ps(v);
	__m128 sums = _mm_add_ps(v, shuf);
	shuf = _mm_movehl_ps(shuf, sums);
	sums = _mm_add_ss(sums, shuf);
	return _mm_cvtss_f32(sums);
}

inline float hsum256_ps_avx(const __m256 v)
{
	__m128 vlow = _mm256_castps256_ps128(v);
	const __m128 vhigh = _mm256_extractf128_ps(v, 1);
	vlow = _mm_add_ps(vlow, vhigh);
	return hsum_ps_sse3(vlow);
}

// Narkowicz 2015, "ACES Filmic Tone Mapping Curve"
inline float3 aces(const float3& x)
{
	return clamp((x * (2.51f * x + 0.03f)) / (x * (2.43f * x + 0.59f) + 0.14f), 0.0f, 1.0f);
}

inline float ease_out_2_quadratic(const float x)
{
	const float invX = 1.0f - x;
	return 1.0f - (invX * invX);
}

inline float ease_out_3_cubic(const float x)
{
	const float invX = 1.0f - x;
	return 1.0f - (invX * invX * invX);
}

inline float ease_out_4_quartic(const float x)
{
	const float invX = 1.0f - x;
	return 1.0f - (invX * invX * invX * invX);
}

inline float ease_out_5_quintic(const float x)
{
	const float invX = 1.0f - x;
	return 1.0f - (invX * invX * invX * invX * invX);
}

inline float ease_out_exponential(const float x)
{
	return 1.0f - powf(2.0f, -10.0f * x);
}

inline float ease_out_logarthmic(const float x)
{
	return logf(1.0f + x * 9.0f) / logf(10.0f);
}

inline float ease_in_exponential(const float x)
{
	return powf(2.0f, 10.0f * x - 10.0f);
}

inline float ease_in_out_exponential(const float x)
{
	if(x == 0.0f) return 0.0f;
	if(x == 1.0f) return 1.0f;
	if(x < 0.5f) return 0.5f * powf(2.0f, (20.0f * x) - 10.0f);
	return 1.0f - 0.5f * powf(2.0f, (-20.0f * x) + 10.0f);
}

inline float ease_in_circular(const float x)
{
	return 1.0f - sqrtf(1.0f - (x * x));
}

inline float ease_in_out_elastic(const float x)
{
	if(x == 0.0f || x == 1.0f) return x;
	constexpr float c5 = (TWOPI) / 4.5f;
	if(x < 0.5f)  return -0.5f * (powf(2.0f, (20.0f * x) - 10.0f) * sinf((x * 20.0f - 11.125f) * c5));
	return powf(2.0f, (-20.0f * x) + 10.0f) * sinf((x * 20.0f - 11.125f) * c5) * 0.5f + 1.0f;
}

inline float ease_out_bounce(float x)
{
	constexpr float n1 = 7.5625f;
	constexpr float d1 = 2.75f;
	if(x < 1.0f / d1) return n1 * x * x;
	if(x < 2.0f / d1) return n1 * (x -= (1.5f / d1)) * x + 0.75f;
	if(x < 2.5f / d1) return n1 * (x -= (2.25f / d1)) * x + 0.9375f;
	return n1 * (x -= (2.625f / d1)) * x + 0.984375f;
}

inline float ease_in_bounce(const float x)
{
	return 1.0f - ease_out_bounce(1.0f - x);
}

inline float ease_in_out_bounce(const float x)
{
	if(x < 0.5f) return ease_in_bounce(x * 2.0f) * 0.5f;
	return ease_out_bounce(x * 2.0f - 1.0f) * 0.5f + 0.5f;
}
