#pragma once

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
