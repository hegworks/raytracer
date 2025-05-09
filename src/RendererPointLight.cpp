﻿#include "precomp.h"

#ifdef SCALAR
float3 Renderer::CalcAllPointLightsScalar(const float3& p, const float3& n, const float3& brdf) const
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const int count = static_cast<int>(scene.m_pointLightList.size());

	for(int i = 0; i < count; ++i)
	{
		const PointLight& light = scene.m_pointLightList[i];

		// vi: incoming light vector
		const float3 vi = light.m_pos - p;

		// t: distance between shadowRayPos and lightPos
		const float t = length(vi);

		// wi: incoming light direction
		const float3 wi = vi / t;

		// lambert's cosine law
		const float cosi = dot(n, wi);
		if(cosi <= 0)
			continue;

		// inverse square law
		const float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;
#endif

		retVal += brdf * light.m_color * light.m_intensity * cosi * falloff;
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsScalar(const float3& p, const float3& n, const float3& brdf, const int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const int numPointLights = static_cast<int>(scene.m_pointLightList.size());
	constexpr int count = STOCH_SAMPLES;

	for(int i = 0; i < count; ++i)
	{
		const uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);
		PointLight& light = scene.m_pointLightList[randIdx];

		// vi: incoming light vector
		float3 vi = light.m_pos - p;

		// t: distance between shadowRayPos and lightPos
		const float t = length(vi);

		// wi: incoming light direction
		float3 wi = vi / t;

		// lambert's cosine law
		const float cosi = dot(n, wi);
		if(cosi <= 0)
			continue;

		// inverse square law
		const float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;
#endif

		retVal += brdf * light.m_color * light.m_intensity * cosi * falloff;
	}

	return retVal;
}
#endif

#ifdef DOD
float3 Renderer::CalcAllPointLightsDOD(const float3& p, const float3& n, const float3& brdf) const
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const int count = scene.npl;

	for(int i = 0; i < count; ++i)
	{
		const uint idx = i;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		const float vix = scene.plx[idx] - p.x;
		const float viy = scene.ply[idx] - p.y;
		const float viz = scene.plz[idx] - p.z;

		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		const float t = sqrt(vix * vix + viy * viy + viz * viz);

		// wi: incoming light direction
		// float3 wi = vi / t;
		const float rcp = 1.0f / t;
		const float wix = vix * rcp;
		const float wiy = viy * rcp;
		const float wiz = viz * rcp;

		// lambert's cosine law
		// float cosi = dot(n,wi)
		const float cosi = n.x * wix + n.y * wiy + n.z * wiz;
		if(cosi <= 0)
			continue;

		// inverse square law
		const float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		const float3 wi = {wix, wiy, wiz};
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;
#endif

		// color with applied brdf
		const float r = brdf.x * scene.plr[idx];
		const float g = brdf.y * scene.plg[idx];
		const float b = brdf.z * scene.plb[idx];

		retVal += float3(r, g, b) * scene.pli[idx] * cosi * falloff;
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsDOD(const float3& p, const float3& n, const float3& brdf, const int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const int numPointLights = scene.npl;
	constexpr int count = STOCH_SAMPLES;

	for(int i = 0; i < count; ++i)
	{
		const uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);
		const uint idx = randIdx;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		const float vix = scene.plx[idx] - p.x;
		const float viy = scene.ply[idx] - p.y;
		const float viz = scene.plz[idx] - p.z;

		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		float t = sqrt(vix * vix + viy * viy + viz * viz);

		// wi: incoming light direction
		// float3 wi = vi / t;
		const float rcp = 1.0f / t;
		float wix = vix * rcp;
		float wiy = viy * rcp;
		float wiz = viz * rcp;

		// lambert's cosine law
		// float cosi = dot(n,wi)
		const float cosi = n.x * wix + n.y * wiy + n.z * wiz;
		if(cosi <= 0)
			continue;

		// inverse square law
		const float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		const float3 wi = {wix, wiy, wiz};
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;
#endif

		// color with applied brdf
		const float r = brdf.x * scene.plr[idx];
		const float g = brdf.y * scene.plg[idx];
		const float b = brdf.z * scene.plb[idx];

		retVal += float3(r, g, b) * scene.pli[idx] * cosi * falloff;
	}

	return retVal;
}
#endif

#ifdef SIMD
float3 Renderer::CalcAllPointLightsSIMD(const float3& p, const float3& n, const float3& brdf) const
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const quadf px4 = {_mm_set_ps1(p.x)};
	const quadf py4 = {_mm_set_ps1(p.y)};
	const quadf pz4 = {_mm_set_ps1(p.z)};

	const __m128 nx4 = _mm_set_ps1(n.x);
	const __m128 ny4 = _mm_set_ps1(n.y);
	const __m128 nz4 = _mm_set_ps1(n.z);

	const __m128 brdfx4 = _mm_set_ps1(brdf.x);
	const __m128 brdfy4 = _mm_set_ps1(brdf.y);
	const __m128 brdfz4 = _mm_set_ps1(brdf.z);

	const int count = scene.npl / 4;

	for(int i = 0; i < count; ++i)
	{
		uint idx = i;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		const __m128 vix4 = _mm_sub_ps(scene.plx4[idx], px4.f4);
		const __m128 viy4 = _mm_sub_ps(scene.ply4[idx], py4.f4);
		const __m128 viz4 = _mm_sub_ps(scene.plz4[idx], pz4.f4);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		const quadf t4 =
		{
			_mm_sqrt_ps
			(
				_mm_add_ps
				(
					_mm_add_ps
					(
						_mm_mul_ps(vix4, vix4), _mm_mul_ps(viy4, viy4)
					)
					, _mm_mul_ps(viz4, viz4)
				)
			)
		};


		// wi: incoming light direction
		// float3 wi = vi / t;
		const __m128 rcp = _mm_rcp_ps(t4.f4);
		const quadf wix4 = {_mm_mul_ps(vix4, rcp)};
		const quadf wiy4 = {_mm_mul_ps(viy4, rcp)};
		const quadf wiz4 = {_mm_mul_ps(viz4, rcp)};


		// lambert's cosine law
		// float cosi = dot(n,wi)
		quadf cosi4 = {
			_mm_add_ps
			(
				_mm_add_ps
				(
					_mm_mul_ps(nx4, wix4.f4), _mm_mul_ps(ny4, wiy4.f4)
				)
				, _mm_mul_ps(nz4, wiz4.f4)
			)};
		cosi4.f4 = _mm_max_ps(cosi4.f4, _mm_setzero_ps());

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		quadf shadowMask = {_mm_set_ps1(1)};
		{
			if(cosi4.f[0] > 0)
			{
				const float3 wi = {wix4.f[0], wiy4.f[0], wiz4.f[0]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[0] - TWO_EPS}))
					shadowMask.f[0] = 0;
			}
		}
		{
			if(cosi4.f[1] > 0)
			{
				const float3 wi = {wix4.f[1], wiy4.f[1], wiz4.f[1]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[1] - TWO_EPS}))
					shadowMask.f[1] = 0;
			}
		}
		{
			if(cosi4.f[2] > 0)
			{
				const float3 wi = {wix4.f[2], wiy4.f[2], wiz4.f[2]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[2] - TWO_EPS}))
					shadowMask.f[2] = 0;
			}
		}
		{
			if(cosi4.f[3] > 0)
			{
				const float3 wi = {wix4.f[3], wiy4.f[3], wiz4.f[3]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[3] - TWO_EPS}))
					shadowMask.f[3] = 0;
			}
		}
		cosi4.f4 = _mm_mul_ps(cosi4.f4, shadowMask.f4);
#endif

		// inverse square law
		//const float falloff = 1.0f / (t * t);
		const __m128 falloff4 = _mm_rcp_ps(_mm_mul_ps(t4.f4, t4.f4));


		// color with applied brdf
		const __m128 r4 = _mm_mul_ps(brdfx4, scene.plr4[idx]);
		const __m128 g4 = _mm_mul_ps(brdfy4, scene.plg4[idx]);
		const __m128 b4 = _mm_mul_ps(brdfz4, scene.plb4[idx]);


		// cosi * falloff * intensity
		const __m128 effect4 =
			_mm_mul_ps
			(
				_mm_mul_ps(scene.pli4[idx], cosi4.f4)
				, falloff4
			);


		// effect4 * rbg4
		const quadf lr4 = {_mm_mul_ps(r4, effect4)};
		const quadf lg4 = {_mm_mul_ps(g4, effect4)};
		const quadf lb4 = {_mm_mul_ps(b4, effect4)};


		const float r = hsum_ps_sse3(lr4.f4);
		const float g = hsum_ps_sse3(lg4.f4);
		const float b = hsum_ps_sse3(lb4.f4);

		retVal += float3(r, g, b);
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsSIMD(const float3& p, const float3& n, const float3& brdf, int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const quadf px4 = {_mm_set_ps1(p.x)};
	const quadf py4 = {_mm_set_ps1(p.y)};
	const quadf pz4 = {_mm_set_ps1(p.z)};

	const __m128 nx4 = _mm_set_ps1(n.x);
	const __m128 ny4 = _mm_set_ps1(n.y);
	const __m128 nz4 = _mm_set_ps1(n.z);

	const __m128 brdfx4 = _mm_set_ps1(brdf.x);
	const __m128 brdfy4 = _mm_set_ps1(brdf.y);
	const __m128 brdfz4 = _mm_set_ps1(brdf.z);

	const int numPointLights = scene.npl / 4;
	const int count = STOCH_SAMPLES / 4;
	uint idx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);

	for(int i = 0; i < count; ++i)
	{
		idx = (idx + i) % numPointLights;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		const __m128 vix4 = _mm_sub_ps(scene.plx4[idx], px4.f4);
		const __m128 viy4 = _mm_sub_ps(scene.ply4[idx], py4.f4);
		const __m128 viz4 = _mm_sub_ps(scene.plz4[idx], pz4.f4);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		const quadf t4 =
		{
			_mm_sqrt_ps
			(
				_mm_add_ps
				(
					_mm_add_ps
					(
						_mm_mul_ps(vix4, vix4), _mm_mul_ps(viy4, viy4)
					)
					, _mm_mul_ps(viz4, viz4)
				)
			)
		};


		// wi: incoming light direction
		// float3 wi = vi / t;
		const __m128 rcp = _mm_rcp_ps(t4.f4);
		const quadf wix4 = {_mm_mul_ps(vix4, rcp)};
		const quadf wiy4 = {_mm_mul_ps(viy4, rcp)};
		const quadf wiz4 = {_mm_mul_ps(viz4, rcp)};


		// lambert's cosine law
		// float cosi = dot(n,wi)
		quadf cosi4 = {
			_mm_add_ps
			(
				_mm_add_ps
				(
					_mm_mul_ps(nx4, wix4.f4), _mm_mul_ps(ny4, wiy4.f4)
				)
				, _mm_mul_ps(nz4, wiz4.f4)
			)};
		//cosi4.f4 = _mm_andnot_ps(_mm_cmple_ps(cosi4.f4, _mm_setzero_ps()), cosi4.f4);
		cosi4.f4 = _mm_max_ps(cosi4.f4, _mm_setzero_ps());

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		quadf shadowMask = {_mm_set_ps1(1)};
		{
			if(cosi4.f[0] > 0)
			{
				const float3 wi = {wix4.f[0], wiy4.f[0], wiz4.f[0]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[0] - TWO_EPS}))
					shadowMask.f[0] = 0;
			}
		}
		{
			if(cosi4.f[1] > 0)
			{
				const float3 wi = {wix4.f[1], wiy4.f[1], wiz4.f[1]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[1] - TWO_EPS}))
					shadowMask.f[1] = 0;
			}
		}
		{
			if(cosi4.f[2] > 0)
			{
				const float3 wi = {wix4.f[2], wiy4.f[2], wiz4.f[2]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[2] - TWO_EPS}))
					shadowMask.f[2] = 0;
			}
		}
		{
			if(cosi4.f[3] > 0)
			{
				const float3 wi = {wix4.f[3], wiy4.f[3], wiz4.f[3]};
				if(scene.IsOccluded({p + wi * EPS, wi, t4.f[3] - TWO_EPS}))
					shadowMask.f[3] = 0;
			}
		}
		cosi4.f4 = _mm_mul_ps(cosi4.f4, shadowMask.f4);
#endif

		// inverse square law
		//const float falloff = 1.0f / (t * t);
		const __m128 falloff4 = _mm_rcp_ps(_mm_mul_ps(t4.f4, t4.f4));


		// color with applied brdf
		const __m128 r4 = _mm_mul_ps(brdfx4, scene.plr4[idx]);
		const __m128 g4 = _mm_mul_ps(brdfy4, scene.plg4[idx]);
		const __m128 b4 = _mm_mul_ps(brdfz4, scene.plb4[idx]);


		// cosi * falloff * intensity
		const __m128 effect4 =
			_mm_mul_ps
			(
				_mm_mul_ps(scene.pli4[idx], cosi4.f4)
				, falloff4
			);


		// effect4 * rbg4
		const quadf lr4 = {_mm_mul_ps(r4, effect4)};
		const quadf lg4 = {_mm_mul_ps(g4, effect4)};
		const quadf lb4 = {_mm_mul_ps(b4, effect4)};


		const float r = hsum_ps_sse3(lr4.f4);
		const float g = hsum_ps_sse3(lg4.f4);
		const float b = hsum_ps_sse3(lb4.f4);

		retVal += float3(r, g, b);
	}

	return retVal;
}

#endif

#ifdef AVX
float3 Renderer::CalcAllPointLightsAVX(const float3& p, const float3& n, const float3& brdf) const
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const octf px8 = {_mm256_set1_ps(p.x)};
	const octf py8 = {_mm256_set1_ps(p.y)};
	const octf pz8 = {_mm256_set1_ps(p.z)};

	const __m256 nx8 = _mm256_set1_ps(n.x);
	const __m256 ny8 = _mm256_set1_ps(n.y);
	const __m256 nz8 = _mm256_set1_ps(n.z);

	const __m256 brdfx8 = _mm256_set1_ps(brdf.x);
	const __m256 brdfy8 = _mm256_set1_ps(brdf.y);
	const __m256 brdfz8 = _mm256_set1_ps(brdf.z);

	const int count = scene.npl / 8;

	for(int i = 0; i < count; ++i)
	{
		uint idx = i;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		const __m256 vix8 = _mm256_sub_ps(scene.plx8[idx], px8.f8);
		const __m256 viy8 = _mm256_sub_ps(scene.ply8[idx], py8.f8);
		const __m256 viz8 = _mm256_sub_ps(scene.plz8[idx], pz8.f8);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		const octf t8 =
		{
			_mm256_sqrt_ps
			(
				_mm256_add_ps
				(
					_mm256_add_ps
					(
						_mm256_mul_ps(vix8, vix8), _mm256_mul_ps(viy8, viy8)
					)
					, _mm256_mul_ps(viz8, viz8)
				)
			)
		};


		// wi: incoming light direction
		// float3 wi = vi / t;
		const __m256 rcp = _mm256_rcp_ps(t8.f8);
		const octf wix8 = {_mm256_mul_ps(vix8, rcp)};
		const octf wiy8 = {_mm256_mul_ps(viy8, rcp)};
		const octf wiz8 = {_mm256_mul_ps(viz8, rcp)};


		// lambert's cosine law
		// float cosi = dot(n,wi)
		octf cosi8 = {
			_mm256_add_ps
			(
				_mm256_add_ps
				(
					_mm256_mul_ps(nx8, wix8.f8), _mm256_mul_ps(ny8, wiy8.f8)
				)
				, _mm256_mul_ps(nz8, wiz8.f8)
			)};
		cosi8.f8 = _mm256_max_ps(cosi8.f8, _mm256_setzero_ps());

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		octf shadowMask = {_mm256_set1_ps(1)};
		if(cosi8.f[0] > 0)
		{
			const float3 wi = {wix8.f[0], wiy8.f[0], wiz8.f[0]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[0] - TWO_EPS}))
				shadowMask.f[0] = 0;
		}
		if(cosi8.f[1] > 0)
		{
			const float3 wi = {wix8.f[1], wiy8.f[1], wiz8.f[1]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[1] - TWO_EPS}))
				shadowMask.f[1] = 0;
		}
		if(cosi8.f[2] > 0)
		{
			const float3 wi = {wix8.f[2], wiy8.f[2], wiz8.f[2]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[2] - TWO_EPS}))
				shadowMask.f[2] = 0;
		}
		if(cosi8.f[3] > 0)
		{
			const float3 wi = {wix8.f[3], wiy8.f[3], wiz8.f[3]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[3] - TWO_EPS}))
				shadowMask.f[3] = 0;
		}
		if(cosi8.f[4] > 0)
		{
			const float3 wi = {wix8.f[4], wiy8.f[4], wiz8.f[4]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[4] - TWO_EPS}))
				shadowMask.f[4] = 0;
		}
		if(cosi8.f[5] > 0)
		{
			const float3 wi = {wix8.f[5], wiy8.f[5], wiz8.f[5]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[5] - TWO_EPS}))
				shadowMask.f[5] = 0;
		}
		if(cosi8.f[6] > 0)
		{
			const float3 wi = {wix8.f[6], wiy8.f[6], wiz8.f[6]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[6] - TWO_EPS}))
				shadowMask.f[6] = 0;
		}
		if(cosi8.f[7] > 0)
		{
			const float3 wi = {wix8.f[7], wiy8.f[7], wiz8.f[7]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[7] - TWO_EPS}))
				shadowMask.f[7] = 0;
		}
		cosi8.f8 = _mm256_mul_ps(cosi8.f8, shadowMask.f8);
#endif

		// inverse square law
		//const float falloff = 1.0f / (t * t);
		const __m256 falloff8 = _mm256_rcp_ps(_mm256_mul_ps(t8.f8, t8.f8));


		// color with applied brdf
		const __m256 r8 = _mm256_mul_ps(brdfx8, scene.plr8[idx]);
		const __m256 g8 = _mm256_mul_ps(brdfy8, scene.plg8[idx]);
		const __m256 b8 = _mm256_mul_ps(brdfz8, scene.plb8[idx]);


		// cosi * falloff * intensity
		const __m256 effect8 =
			_mm256_mul_ps
			(
				_mm256_mul_ps(scene.pli8[idx], cosi8.f8)
				, falloff8
			);


		// effect8 * rbg8
		const octf lr8 = {_mm256_mul_ps(r8, effect8)};
		const octf lg8 = {_mm256_mul_ps(g8, effect8)};
		const octf lb8 = {_mm256_mul_ps(b8, effect8)};


		const float r = hsum256_ps_avx(lr8.f8);
		const float g = hsum256_ps_avx(lg8.f8);
		const float b = hsum256_ps_avx(lb8.f8);

		retVal += float3(r, g, b);
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsAVX(const float3& p, const float3& n, const float3& brdf, int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const octf px8 = {_mm256_set1_ps(p.x)};
	const octf py8 = {_mm256_set1_ps(p.y)};
	const octf pz8 = {_mm256_set1_ps(p.z)};

	const __m256 nx8 = _mm256_set1_ps(n.x);
	const __m256 ny8 = _mm256_set1_ps(n.y);
	const __m256 nz8 = _mm256_set1_ps(n.z);

	const __m256 brdfx8 = _mm256_set1_ps(brdf.x);
	const __m256 brdfy8 = _mm256_set1_ps(brdf.y);
	const __m256 brdfz8 = _mm256_set1_ps(brdf.z);

	const int numPointLights = scene.npl / 8;
	constexpr int count = STOCH_SAMPLES / 8;
	uint idx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);

	for(int i = 0; i < count; ++i)
	{
		idx = (idx + i) % (numPointLights);

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		const __m256 vix8 = _mm256_sub_ps(scene.plx8[idx], px8.f8);
		const __m256 viy8 = _mm256_sub_ps(scene.ply8[idx], py8.f8);
		const __m256 viz8 = _mm256_sub_ps(scene.plz8[idx], pz8.f8);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		const octf t8 =
		{
			_mm256_sqrt_ps
			(
				_mm256_add_ps
				(
					_mm256_add_ps
					(
						_mm256_mul_ps(vix8, vix8), _mm256_mul_ps(viy8, viy8)
					)
					, _mm256_mul_ps(viz8, viz8)
				)
			)
		};


		// wi: incoming light direction
		// float3 wi = vi / t;
		const __m256 rcp = _mm256_rcp_ps(t8.f8);
		const octf wix8 = {_mm256_mul_ps(vix8, rcp)};
		const octf wiy8 = {_mm256_mul_ps(viy8, rcp)};
		const octf wiz8 = {_mm256_mul_ps(viz8, rcp)};


		// lambert's cosine law
		// float cosi = dot(n,wi)
		octf cosi8 = {
			_mm256_add_ps
			(
				_mm256_add_ps
				(
					_mm256_mul_ps(nx8, wix8.f8), _mm256_mul_ps(ny8, wiy8.f8)
				)
				, _mm256_mul_ps(nz8, wiz8.f8)
			)};
		cosi8.f8 = _mm256_max_ps(cosi8.f8, _mm256_setzero_ps());

#if !defined(SIMD_TEST_SCENE) || (defined(SIMD_TEST_SCENE) && defined(SHADOWRAY))
		// shadow ray
		octf shadowMask = {_mm256_set1_ps(1)};
		if(cosi8.f[0] > 0)
		{
			const float3 wi = {wix8.f[0], wiy8.f[0], wiz8.f[0]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[0] - TWO_EPS}))
				shadowMask.f[0] = 0;
		}
		if(cosi8.f[1] > 0)
		{
			const float3 wi = {wix8.f[1], wiy8.f[1], wiz8.f[1]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[1] - TWO_EPS}))
				shadowMask.f[1] = 0;
		}
		if(cosi8.f[2] > 0)
		{
			const float3 wi = {wix8.f[2], wiy8.f[2], wiz8.f[2]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[2] - TWO_EPS}))
				shadowMask.f[2] = 0;
		}
		if(cosi8.f[3] > 0)
		{
			const float3 wi = {wix8.f[3], wiy8.f[3], wiz8.f[3]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[3] - TWO_EPS}))
				shadowMask.f[3] = 0;
		}
		if(cosi8.f[4] > 0)
		{
			const float3 wi = {wix8.f[4], wiy8.f[4], wiz8.f[4]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[4] - TWO_EPS}))
				shadowMask.f[4] = 0;
		}
		if(cosi8.f[5] > 0)
		{
			const float3 wi = {wix8.f[5], wiy8.f[5], wiz8.f[5]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[5] - TWO_EPS}))
				shadowMask.f[5] = 0;
		}
		if(cosi8.f[6] > 0)
		{
			const float3 wi = {wix8.f[6], wiy8.f[6], wiz8.f[6]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[6] - TWO_EPS}))
				shadowMask.f[6] = 0;
		}
		if(cosi8.f[7] > 0)
		{
			const float3 wi = {wix8.f[7], wiy8.f[7], wiz8.f[7]};
			if(scene.IsOccluded({p + wi * EPS, wi, t8.f[7] - TWO_EPS}))
				shadowMask.f[7] = 0;
		}
		cosi8.f8 = _mm256_mul_ps(cosi8.f8, shadowMask.f8);
#endif

		// inverse square law
		//const float falloff = 1.0f / (t * t);
		const __m256 falloff8 = _mm256_rcp_ps(_mm256_mul_ps(t8.f8, t8.f8));


		// color with applied brdf
		const __m256 r8 = _mm256_mul_ps(brdfx8, scene.plr8[idx]);
		const __m256 g8 = _mm256_mul_ps(brdfy8, scene.plg8[idx]);
		const __m256 b8 = _mm256_mul_ps(brdfz8, scene.plb8[idx]);


		// cosi * falloff * intensity
		const __m256 effect8 =
			_mm256_mul_ps
			(
				_mm256_mul_ps(scene.pli8[idx], cosi8.f8)
				, falloff8
			);


		// effect8 * rbg8
		const octf lr8 = {_mm256_mul_ps(r8, effect8)};
		const octf lg8 = {_mm256_mul_ps(g8, effect8)};
		const octf lb8 = {_mm256_mul_ps(b8, effect8)};


		const float r = hsum256_ps_avx(lr8.f8);
		const float g = hsum256_ps_avx(lg8.f8);
		const float b = hsum256_ps_avx(lb8.f8);

		retVal += float3(r, g, b);
	}

	return retVal;
}
#endif
