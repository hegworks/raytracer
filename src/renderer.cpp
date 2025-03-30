#include "precomp.h"

#include <common.h>

#include "DBG.h"
#include "Material.h"
#include "Model.h"
#include "PointLight.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	// create fp32 rgb pixel buffer to render to
	accumulator = (float4*)MALLOC64(SCRWIDTH * SCRHEIGHT * 16);
	memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);

	illuminations = (float4*)MALLOC64(SCRWIDTH * SCRHEIGHT * 16);
	memset(illuminations, 0, SCRWIDTH * SCRHEIGHT * 16);

	for(int y = 0; y < SCRHEIGHT; y++)
	{
		for(int x = 0; x < SCRWIDTH; x++)
		{
			pixelSeeds[x + y * SCRWIDTH] = threadRng.InitSeed(1 + x + y * SCRWIDTH);
		}
	}

	acmCounter = 1;

	/*QuadLight& ql = scene.CreateQuadLight();
	ql.m_quad.size = 5;
	ql.m_intensity = 5;
	ql.m_quad.m_pos = {0,5,0};
	ql.m_quad.T = mat4::Translate(ql.m_quad.m_pos);
	ql.m_quad.invT = ql.m_quad.T.FastInvertedTransformNoScale();*/

#if _DEBUG
	dbgScrRangeX = {(SCRWIDTH / 2) - 150,(SCRWIDTH / 2) + 150};
	dbgScrRangeY = {(SCRHEIGHT / 2) - 150,(SCRHEIGHT / 2) + 150};
	dbgScrRangeY = {(SCRHEIGHT / 2) - 150,(SCRHEIGHT / 2) + 150};
#endif // _DEBUG
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(float deltaTime)
{
	// pixel loop
	Timer t;
	const float scale = 1.0f / static_cast<float>(acmCounter++);

	if(isDbgPixel && !isDbgPixelEntered)
	{
		if(!isDbgPixelClicked)
		{

			dbgpixel = {mousePos.x,mousePos.y};
			return;
		}

		return;
	}

	if(tdd) screen->Clear(0);

	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for(int y = dbgScrRangeY.x; y < dbgScrRangeY.y; y++)
	{
		int yTimesSCRWDTH = y * SCRWIDTH;
		bool tddIsPixelY = tdd && y == tddSliceY;
		// trace a primary ray for each pixel on the line
		for(int x = dbgScrRangeX.x; x < dbgScrRangeX.y; x++)
		{
			if(isDbgPixel && isDbgPixelEntered && dbgpixel.x == x && dbgpixel.y == y) __debugbreak();
			bool tddIsPixelX = tdd && ((!tddSXM && x % tddrx == 0) || (tddSXM && x == tddSXX));
			int pixelIndex = x + yTimesSCRWDTH;
			if(isDbgFixSeed) lastPixelSeeds[pixelIndex] = pixelSeeds[pixelIndex];
			const float xOffset = useAA ? threadRng.RandomFloat(pixelSeeds[pixelIndex]) : 0.0f;
			const float yOffset = useAA ? threadRng.RandomFloat(pixelSeeds[pixelIndex]) : 0.0f;
			float3 stochasticDOFTraced(0);
			for(int s = 0; s < spp; ++s)
			{
				float2 defocusRand = threadRng.RandomPointOnCircle(pixelSeeds[pixelIndex]);
				Ray r = camera.GetPrimaryRay(static_cast<float>(x) + xOffset, static_cast<float>(y) + yOffset, defocusRand);
				stochasticDOFTraced += Trace(r, pixelIndex, 0, tddIsPixelX, tddIsPixelY);
			}
			float3 traced = stochasticDOFTraced / (float)spp;
			if(dot(traced, traced) > dbgFF * dbgFF) traced = dbgFF * normalize(traced); // firefly suppressor
			accumulator[pixelIndex] += float4(traced, 0);
			float4 avg = accumulator[pixelIndex] * scale;
			if(tdd && tddBBG || tdd && screen->pixels[pixelIndex] != 0x0) continue;
			float4 gammaCorrected = float4(sqrtf(avg.x), sqrtf(avg.y), sqrtf(avg.z), 1);
			screen->pixels[pixelIndex] = RGBF32_to_RGB8(&gammaCorrected);
			illuminations[pixelIndex] = gammaCorrected;
			if(isDbgFixSeed) pixelSeeds[pixelIndex] = lastPixelSeeds[pixelIndex];
		}
	}
	if(dbgCalcSum)
	{
		sum = 0;
		for(int i = 0; i < SCRWIDTH * SCRHEIGHT; ++i)
		{
			sum += (illuminations[i].x + illuminations[i].y + illuminations[i].z) / 3.0f;
		}
		sum /= 1000;
	}

	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if(alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / (avg * 1000);
	dfps = fps, drps = rps, davg = avg;
	//printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps);
	// handle user input
	if(camera.HandleInput(deltaTime) || !useACM)
	{
		memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
		acmCounter = 1;
	}

	if(tdd)
	{
		screen->Line(0, 360, SCRWIDTH - 1, 360, 0xff0000);
	}
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace(Ray& ray, int pixelIndex, int depth, bool tddIsPixelX, bool tddIsPixelY)
{
	if(depth == maxDepth)
	{
		return 0; // or a fancy sky color
	}

	scene.Intersect(ray);
	bool hasHit = ray.hit.t < BVH_FAR;
	if(!hasHit)
	{
		if(useSD)
			return scene.SampleSky(ray);
		return 0;
	}

	float3 p = ray.O + ray.hit.t * ray.D; /// intersection point
	float3 n = scene.GetSmoothNormal(ray);
	bool inside = dot(ray.D, scene.GetRawNormal(ray)) > 0.0f;
	if(inside) n = -n;

	bool tddIsCameraY = tdd && IsCloseF(p.y, camera.camPos.y);
	TDDP(ray, p, n, screen, depth, tddIsPixelX, tddIsPixelY, tddIsCameraY);

	float3 l(0);
	Model& model = scene.GetModel(ray);
	Material mat = scene.GetMaterial(ray);
	float3 albedo = model.m_modelData.m_surfaceList.empty() ? mat.m_albedo : scene.GetAlbedo(ray, model);
	float3 brdf = albedo / PI; // for diffuse (matte) surfaces
	switch(mat.m_type)
	{
		case Material::Type::DIFFUSE:
		{
			l += CalcLights(ray, p, n, brdf, pixelIndex, tddIsPixelX, tddIsPixelY, tddIsCameraY);
			break;
		}
		case Material::Type::GLOSSY:
		{
			float3 reflectDir = reflect(ray.D, n);
			Ray reflectRay(p + reflectDir * EPS, reflectDir);
			l += albedo * Trace(reflectRay, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			break;
		}
		case Material::Type::REFRACTIVE:
		{
			float density = mat.m_factor0;
			float ior = mat.m_factor1;
			float3 localN = n;
			if(inside) localN = -n;
			float fres;
			fresnel(ray.D, localN, ior, fres);
			float oneMinusFres = 1.0f - fres;

			if(fres > EPS || oneMinusFres > EPS)
			{
				if(dbgSF)
				{
					bool reflectTrueRefractFalse = threadRng.RandomFloat(pixelSeeds[pixelIndex]) < fres;
					float3 localL(0);
					if(reflectTrueRefractFalse && fres > EPS)
					{
						float3 reflecDir = reflect(ray.D, localN);
						Ray reflecR(p + reflecDir * EPS, reflecDir);
						localL = Trace(reflecR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
					}
					else if(!reflectTrueRefractFalse && oneMinusFres > EPS)
					{
						float3 refracDir = refract(ray.D, localN, ior);
						Ray refracR(p + refracDir * EPS, refracDir);
						localL = Trace(refracR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
					}
					float3 beer = inside ? expf(-mat.m_albedo * density * ray.hit.t) : 1.0f;
					float3 alb = dbgBeer ? beer : mat.m_albedo;
					l += alb * localL;
				}
				else
				{
					float3 reflected(0);
					if(fres > EPS)
					{
						float3 reflecDir = reflect(ray.D, localN);
						Ray reflecR(p + reflecDir * EPS, reflecDir);
						reflected = Trace(reflecR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
					}
					float3 refracted(0);
					if(oneMinusFres > EPS)
					{
						float3 refracDir = refract(ray.D, localN, ior);
						Ray refracR(p + refracDir * EPS, refracDir);
						refracted = Trace(refracR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
					}
					float3 beer = inside ? expf(-mat.m_albedo * density * ray.hit.t) : 1.0f;
					float3 alb = dbgBeer ? beer : mat.m_albedo;
					l += alb * ((fres * reflected) + ((1.0f - fres) * refracted));
				}
			}
			break;
		}
		case Material::Type::PATH_TRACED:
		{
			float smoothness = mat.m_factor0;
			float specularity = mat.m_factor1;

			float3 diffuseDir = normalize(n + threadRng.RandomPointOnHemisphere(pixelSeeds[pixelIndex], n));

			float3 finalDir(0);
			float3 finalMatColor(0);
			bool isSpecularRay = specularity > threadRng.RandomFloat(pixelSeeds[pixelIndex]);
			if(isSpecularRay)
			{
				float3 specularDir = reflect(ray.D, n);
				finalDir = lerp(diffuseDir, specularDir, smoothness);
				finalMatColor = 1.0f; // No Effect when multipiled by Trace()
			}
			else
			{
				finalDir = diffuseDir;
				finalMatColor = brdf;
			}

			Ray finalRay(p + finalDir * EPS, finalDir);

			float3 finalTrace = finalMatColor * Trace(finalRay, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);

			float3 directLight = CalcLights(ray, p, n, brdf, pixelIndex, tddIsPixelX, tddIsPixelY, tddIsCameraY);

			l += finalTrace + directLight;

			break;
		}
		case Material::Type::EMISSIVE:
		{
			float intensity = mat.m_factor0;
			l += mat.m_albedo * intensity;
			break;
		}
	}

	switch(ndal)
	{
		case 0:
			return hasHit ? (n + 1.0f) * 0.5f : 0;
		case 1:
			return float3(ray.hit.t) * 0.1f;
		case 2:
			return albedo;
		case 3:
			return l;
		default:
			throw std::runtime_error("Unhandled situation");
	}
}

float3 Renderer::CalcLights([[maybe_unused]] Ray& ray, float3 p, float3 n, float3 brdf, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, bool isTddCameraY)
{
#ifdef STOCH
	int numSamples = STOCH_SAMPLES;
	float3 stochasticL(0);
	const uint numSpotLights = static_cast<uint>(scene.m_spotLightList.size());
	const uint numDirLights = static_cast<uint>(scene.m_dirLightList.size());
	const uint numQuadLights = static_cast<uint>(scene.m_quadLightList.size());
#ifdef SCALAR
	const uint numPointLights = static_cast<uint>(scene.m_pointLightList.size());
#elif defined(DOD) || defined(SIMD) || defined(AVX)
	const uint numPointLights = scene.npl;
#endif

	const uint numLights = numPointLights + numSpotLights + numDirLights + numQuadLights;

	if(numLights == 0) return 0;

	if(numPointLights > 0)
	{
#ifdef SCALAR
		stochasticL += CalcStochPointLightsScalar(p, n, brdf, pixelIndex);
#elif defined(DOD)
		stochasticL += CalcStochPointLightsDOD(p, n, brdf, pixelIndex);
#elif defined(SIMD)
		stochasticL += CalcStochPointLightsSIMD(p, n, brdf, pixelIndex);
#elif defined(AVX) 
		stochasticL += CalcStochPointLightsAVX(p, n, brdf, pixelIndex);
#endif
	}

	if(numSpotLights > 0)
	{
		for(int i = 0; i < numSamples; ++i)
		{
			uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], numPointLights, numPointLights + numSpotLights);

			SpotLight& light = scene.m_spotLightList[randIdx - numPointLights];
			stochasticL += CalcSpotLight(light, p, n, brdf);
		}
	}

	if(numDirLights > 0)
	{
		for(int i = 0; i < numSamples; ++i)
		{
			uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], numPointLights + numSpotLights, numPointLights + numSpotLights + numDirLights);

			DirLight& light = scene.m_dirLightList[randIdx - numPointLights - numSpotLights];
			stochasticL += CalclDirLight(light, p, n, brdf);
		}
	}

	if(numQuadLights > 0)
	{
		for(int i = 0; i < numSamples; ++i)
		{
			uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], numPointLights + numSpotLights + numDirLights, numLights);

			QuadLight& light = scene.m_quadLightList[randIdx - numPointLights - numSpotLights - numDirLights];
			stochasticL += CalcQuadLight(light, p, n, brdf, pixelIndex);
		}
	}

	return stochasticL * ((float)numLights / (float)numSamples);

#else
	float3 l(0); /// total outgoing radiance
#ifdef SCALAR
	l += CalcAllPointLightsScalar(p, n, brdf);
#elif defined(DOD)
	l += CalcAllPointLightsDOD(p, n, brdf);
#elif defined(SIMD)
	l += CalcAllPointLightsSIMD(p, n, brdf);
#elif defined(AVX)
	l += CalcAllPointLightsAVX(p, n, brdf);
#endif
	l += CalcAllSpotLights(p, n, brdf);
	l += CalcAllDirLights(p, n, brdf);
	l += CalcAllQuadLights(p, n, brdf, pixelIndex);

	return l;
#endif
}

#ifdef SCALAR
float3 Renderer::CalcAllPointLightsScalar(float3 p, float3 n, float3 brdf)
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

		// shadow ray
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;

		// inverse square law
		const float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

		retVal += brdf * light.m_color * light.m_intensity * cosi * falloff;
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsScalar(float3 p, float3 n, float3 brdf, int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const int numPointLights = static_cast<int>(scene.m_pointLightList.size());
	const int count = STOCH_SAMPLES;

	for(int i = 0; i < count; ++i)
	{
		uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);
		PointLight& light = scene.m_pointLightList[randIdx];

		// vi: incoming light vector
		float3 vi = light.m_pos - p;

		// t: distance between shadowRayPos and lightPos
		float t = length(vi);

		// wi: incoming light direction
		float3 wi = vi / t;

		// lambert's cosine law
		float cosi = dot(n, wi);
		if(cosi <= 0)
			continue;

		// shadow ray
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;

		// inverse square law
		float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

		retVal += brdf * light.m_color * light.m_intensity * cosi * falloff;
	}

	return retVal;
}
#endif

#ifdef DOD
float3 Renderer::CalcAllPointLightsDOD(float3 p, float3 n, float3 brdf)
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

		// shadow ray
		const float3 wi = {wix, wiy, wiz};
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;

		// inverse square law
		const float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

		// color with applied brdf
		const float r = brdf.x * scene.plr[idx];
		const float g = brdf.y * scene.plg[idx];
		const float b = brdf.z * scene.plb[idx];

		retVal += float3(r, g, b) * scene.pli[idx] * cosi * falloff;
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsDOD(float3 p, float3 n, float3 brdf, int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	const int numPointLights = scene.npl;
	const int count = STOCH_SAMPLES;

	for(int i = 0; i < count; ++i)
	{
		uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);
		uint idx = randIdx;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		float vix = scene.plx[idx] - p.x;
		float viy = scene.ply[idx] - p.y;
		float viz = scene.plz[idx] - p.z;

		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		float t = sqrt(vix * vix + viy * viy + viz * viz);

		// wi: incoming light direction
		// float3 wi = vi / t;
		float rcp = 1.0f / t;
		float wix = vix * rcp;
		float wiy = viy * rcp;
		float wiz = viz * rcp;

		// lambert's cosine law
		// float cosi = dot(n,wi)
		float cosi = n.x * wix + n.y * wiy + n.z * wiz;
		if(cosi <= 0)
			continue;

		// shadow ray
		const float3 wi = {wix, wiy, wiz};
		Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
		if(scene.IsOccluded(shadowRay))
			continue;

		// inverse square law
		float falloff = 1.0f / (t * t);
		if(falloff < EPS)
			continue;

		// color with applied brdf
		float r = brdf.x * scene.plr[idx];
		float g = brdf.y * scene.plg[idx];
		float b = brdf.z * scene.plb[idx];

		retVal += float3(r, g, b) * scene.pli[idx] * cosi * falloff;
	}

	return retVal;
}
#endif

#ifdef SIMD
float3 Renderer::CalcAllPointLightsSIMD(float3 p, float3 n, float3 brdf)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	quadf px4 = {_mm_set_ps1(p.x)};
	quadf py4 = {_mm_set_ps1(p.y)};
	quadf pz4 = {_mm_set_ps1(p.z)};

	__m128 nx4 = _mm_set_ps1(n.x);
	__m128 ny4 = _mm_set_ps1(n.y);
	__m128 nz4 = _mm_set_ps1(n.z);

	__m128 brdfx4 = _mm_set_ps1(brdf.x);
	__m128 brdfy4 = _mm_set_ps1(brdf.y);
	__m128 brdfz4 = _mm_set_ps1(brdf.z);

	const int count = scene.npl / 4;

	for(int i = 0; i < count; ++i)
	{
		uint idx = i;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		__m128 vix4 = _mm_sub_ps(scene.plx4[idx], px4.f4);
		__m128 viy4 = _mm_sub_ps(scene.ply4[idx], py4.f4);
		__m128 viz4 = _mm_sub_ps(scene.plz4[idx], pz4.f4);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		quadf t4 =
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
		__m128 rcp = _mm_rcp_ps(t4.f4);
		quadf wix4 = {_mm_mul_ps(vix4, rcp)};
		quadf wiy4 = {_mm_mul_ps(viy4, rcp)};
		quadf wiz4 = {_mm_mul_ps(viz4, rcp)};


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


		// inverse square law
		//const float falloff = 1.0f / (t * t);
		__m128 falloff4 = _mm_rcp_ps(_mm_mul_ps(t4.f4, t4.f4));


		// color with applied brdf
		__m128 r4 = _mm_mul_ps(brdfx4, scene.plr4[idx]);
		__m128 g4 = _mm_mul_ps(brdfy4, scene.plg4[idx]);
		__m128 b4 = _mm_mul_ps(brdfz4, scene.plb4[idx]);


		// cosi * falloff * intensity
		__m128 effect4 =
			_mm_mul_ps
			(
				_mm_mul_ps(scene.pli4[idx], cosi4.f4)
				, falloff4
			);


		// effect4 * rbg4
		quadf lr4 = {_mm_mul_ps(r4, effect4)};
		quadf lg4 = {_mm_mul_ps(g4, effect4)};
		quadf lb4 = {_mm_mul_ps(b4, effect4)};


		float r = hsum_ps_sse3(lr4.f4);
		float g = hsum_ps_sse3(lg4.f4);
		float b = hsum_ps_sse3(lb4.f4);

		retVal += float3(r, g, b);
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsSIMD(float3 p, float3 n, float3 brdf, int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	quadf px4 = {_mm_set_ps1(p.x)};
	quadf py4 = {_mm_set_ps1(p.y)};
	quadf pz4 = {_mm_set_ps1(p.z)};

	__m128 nx4 = _mm_set_ps1(n.x);
	__m128 ny4 = _mm_set_ps1(n.y);
	__m128 nz4 = _mm_set_ps1(n.z);

	__m128 brdfx4 = _mm_set_ps1(brdf.x);
	__m128 brdfy4 = _mm_set_ps1(brdf.y);
	__m128 brdfz4 = _mm_set_ps1(brdf.z);

	const int numPointLights = scene.npl / 4;
	const int count = STOCH_SAMPLES / 4;
	uint idx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);

	for(int i = 0; i < count; ++i)
	{
		idx = (idx + i) % numPointLights;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		__m128 vix4 = _mm_sub_ps(scene.plx4[idx], px4.f4);
		__m128 viy4 = _mm_sub_ps(scene.ply4[idx], py4.f4);
		__m128 viz4 = _mm_sub_ps(scene.plz4[idx], pz4.f4);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		quadf t4 =
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
		__m128 rcp = _mm_rcp_ps(t4.f4);
		quadf wix4 = {_mm_mul_ps(vix4, rcp)};
		quadf wiy4 = {_mm_mul_ps(viy4, rcp)};
		quadf wiz4 = {_mm_mul_ps(viz4, rcp)};


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


		// inverse square law
		//const float falloff = 1.0f / (t * t);
		__m128 falloff4 = _mm_rcp_ps(_mm_mul_ps(t4.f4, t4.f4));


		// color with applied brdf
		__m128 r4 = _mm_mul_ps(brdfx4, scene.plr4[idx]);
		__m128 g4 = _mm_mul_ps(brdfy4, scene.plg4[idx]);
		__m128 b4 = _mm_mul_ps(brdfz4, scene.plb4[idx]);


		// cosi * falloff * intensity
		__m128 effect4 =
			_mm_mul_ps
			(
				_mm_mul_ps(scene.pli4[idx], cosi4.f4)
				, falloff4
			);


		// effect4 * rbg4
		quadf lr4 = {_mm_mul_ps(r4, effect4)};
		quadf lg4 = {_mm_mul_ps(g4, effect4)};
		quadf lb4 = {_mm_mul_ps(b4, effect4)};


		float r = hsum_ps_sse3(lr4.f4);
		float g = hsum_ps_sse3(lg4.f4);
		float b = hsum_ps_sse3(lb4.f4);

		retVal += float3(r, g, b);
	}

	return retVal;
}

#endif


#ifdef AVX
float3 Renderer::CalcAllPointLightsAVX(float3 p, float3 n, float3 brdf)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	octf px8 = {_mm256_set1_ps(p.x)};
	octf py8 = {_mm256_set1_ps(p.y)};
	octf pz8 = {_mm256_set1_ps(p.z)};

	__m256 nx8 = _mm256_set1_ps(n.x);
	__m256 ny8 = _mm256_set1_ps(n.y);
	__m256 nz8 = _mm256_set1_ps(n.z);

	__m256 brdfx8 = _mm256_set1_ps(brdf.x);
	__m256 brdfy8 = _mm256_set1_ps(brdf.y);
	__m256 brdfz8 = _mm256_set1_ps(brdf.z);

	const int count = scene.npl / 8;

	for(int i = 0; i < count; ++i)
	{
		uint idx = i;

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		__m256 vix8 = _mm256_sub_ps(scene.plx8[idx], px8.f8);
		__m256 viy8 = _mm256_sub_ps(scene.ply8[idx], py8.f8);
		__m256 viz8 = _mm256_sub_ps(scene.plz8[idx], pz8.f8);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		octf t8 =
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
		__m256 rcp = _mm256_rcp_ps(t8.f8);
		octf wix8 = {_mm256_mul_ps(vix8, rcp)};
		octf wiy8 = {_mm256_mul_ps(viy8, rcp)};
		octf wiz8 = {_mm256_mul_ps(viz8, rcp)};


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


		// inverse square law
		//const float falloff = 1.0f / (t * t);
		__m256 falloff8 = _mm256_rcp_ps(_mm256_mul_ps(t8.f8, t8.f8));


		// color with applied brdf
		__m256 r8 = _mm256_mul_ps(brdfx8, scene.plr8[idx]);
		__m256 g8 = _mm256_mul_ps(brdfy8, scene.plg8[idx]);
		__m256 b8 = _mm256_mul_ps(brdfz8, scene.plb8[idx]);


		// cosi * falloff * intensity
		__m256 effect8 =
			_mm256_mul_ps
			(
				_mm256_mul_ps(scene.pli8[idx], cosi8.f8)
				, falloff8
			);


		// effect8 * rbg8
		octf lr8 = {_mm256_mul_ps(r8, effect8)};
		octf lg8 = {_mm256_mul_ps(g8, effect8)};
		octf lb8 = {_mm256_mul_ps(b8, effect8)};


		float r = hsum256_ps_avx(lr8.f8);
		float g = hsum256_ps_avx(lg8.f8);
		float b = hsum256_ps_avx(lb8.f8);

		retVal += float3(r, g, b);
	}

	return retVal;
}

float3 Renderer::CalcStochPointLightsAVX(float3 p, float3 n, float3 brdf, int pixelIndex)
{
	PROFILE_FUNCTION();

	float3 retVal(0);

	octf px8 = {_mm256_set1_ps(p.x)};
	octf py8 = {_mm256_set1_ps(p.y)};
	octf pz8 = {_mm256_set1_ps(p.z)};

	__m256 nx8 = _mm256_set1_ps(n.x);
	__m256 ny8 = _mm256_set1_ps(n.y);
	__m256 nz8 = _mm256_set1_ps(n.z);

	__m256 brdfx8 = _mm256_set1_ps(brdf.x);
	__m256 brdfy8 = _mm256_set1_ps(brdf.y);
	__m256 brdfz8 = _mm256_set1_ps(brdf.z);

	const int numPointLights = scene.npl / 8;
	const int count = STOCH_SAMPLES / 8;
	uint idx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numPointLights);

	for(int i = 0; i < count; ++i)
	{
		idx = (idx + i) % (numPointLights);

		// vi: incoming light vector
		// float3 vi = light.m_pos - p;
		__m256 vix8 = _mm256_sub_ps(scene.plx8[idx], px8.f8);
		__m256 viy8 = _mm256_sub_ps(scene.ply8[idx], py8.f8);
		__m256 viz8 = _mm256_sub_ps(scene.plz8[idx], pz8.f8);


		// t: distance between shadowRayPos and lightPos
		// float t = length(vi);
		octf t8 =
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
		__m256 rcp = _mm256_rcp_ps(t8.f8);
		octf wix8 = {_mm256_mul_ps(vix8, rcp)};
		octf wiy8 = {_mm256_mul_ps(viy8, rcp)};
		octf wiz8 = {_mm256_mul_ps(viz8, rcp)};


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


		// inverse square law
		//const float falloff = 1.0f / (t * t);
		__m256 falloff8 = _mm256_rcp_ps(_mm256_mul_ps(t8.f8, t8.f8));


		// color with applied brdf
		__m256 r8 = _mm256_mul_ps(brdfx8, scene.plr8[idx]);
		__m256 g8 = _mm256_mul_ps(brdfy8, scene.plg8[idx]);
		__m256 b8 = _mm256_mul_ps(brdfz8, scene.plb8[idx]);


		// cosi * falloff * intensity
		__m256 effect8 =
			_mm256_mul_ps
			(
				_mm256_mul_ps(scene.pli8[idx], cosi8.f8)
				, falloff8
			);


		// effect8 * rbg8
		octf lr8 = {_mm256_mul_ps(r8, effect8)};
		octf lg8 = {_mm256_mul_ps(g8, effect8)};
		octf lb8 = {_mm256_mul_ps(b8, effect8)};


		float r = hsum256_ps_avx(lr8.f8);
		float g = hsum256_ps_avx(lg8.f8);
		float b = hsum256_ps_avx(lb8.f8);

		retVal += float3(r, g, b);
	}

	return retVal;
}
#endif

float3 Renderer::CalcSpotLight(const SpotLight& light, float3 p, float3 n, float3 brdf)
{
	float3 lPos = light.m_pos; /// LightPos
	float3 vi = lPos - p; /// Light Vector
	float3 wi = normalize(vi); /// incoming light direction
	float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
	float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

	Ray shadowRay(srPos, wi, tMax);
	bool isInShadow = scene.IsOccluded(shadowRay);
	if(isInShadow)
	{
		return 0;
	}

	float cosi = dot(n, wi); /// Lambert's cosine law.
	if(cosi <= 0)
	{
		return 0;
	}

	float falloff = 1.0f / (tMax * tMax); /// inverse square law
	if(falloff < EPS)
	{
		return 0;
	}

	float cutoff = clamp((-dot(wi, light.m_dir) - light.m_cosO) / (light.m_cosI - light.m_cosO), 0.0f, 1.0f); // lerp. NOTICE the minus before dot
	if(cutoff <= 0)
	{
		return 0;
	}

	return brdf * light.m_color * light.m_intensity * cosi * falloff * cutoff * cutoff;
}

float3 Renderer::CalcAllSpotLights(float3 p, float3 n, float3 brdf)
{
	float3 l(0);
	int numSpotLights = static_cast<int>(scene.m_spotLightList.size());
	for(int i = 0; i < numSpotLights; ++i)
	{
		l += CalcSpotLight(scene.m_spotLightList[i], p, n, brdf);
	}
	return l;
}

float3 Renderer::CalclDirLight(const DirLight& light, float3 p, float3 n, float3 brdf)
{
	float3 wi = -light.m_dir; /// incoming light direction
	float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)

	Ray shadowRay(srPos, wi);
	bool isInShadow = scene.IsOccluded(shadowRay);
	if(isInShadow)
	{
		return 0;
	}

	float cosi = dot(n, wi); /// Lambert's cosine law
	if(cosi <= 0)
	{
		return 0;
	}

	return brdf * light.m_color * light.m_intensity * cosi;
}

float3 Renderer::CalcAllDirLights(float3 p, float3 n, float3 brdf)
{
	float3 l(0);
	int numDirLights = static_cast<int>(scene.m_dirLightList.size());
	for(int i = 0; i < numDirLights; ++i)
	{
		l += CalclDirLight(scene.m_dirLightList[i], p, n, brdf);
	}
	return l;
}

float3 Renderer::CalcQuadLight(const QuadLight& light, float3 p, float3 n, float3 brdf, uint pixelIndex)
{
	float3 lSamples = float3(0);
	float3 lightDir = -light.m_quad.GetNormal();
	float pdfEffect = 1 / light.GetPDF();

	for(int j = 0; j < qlNumSamples; ++j)
	{
		float3 a = light.GetRandomPoint(pixelSeeds[pixelIndex]);
		float3 vi = a - p;
		float3 wi = normalize(vi);
		float tMax = length(vi) - EPS * 2;

		if(qlOneSided)
		{
			bool isOppositeSide = dot(lightDir, wi) <= 0;
			if(isOppositeSide)
			{
				continue;
			}
		}

		float3 srPos = p + wi * EPS;
		Ray shadowRay(srPos, wi, tMax);
		bool isInShadow = scene.IsOccluded(shadowRay);
		if(isInShadow)
		{
			continue;
		}

		float cosi = dot(n, wi); /// Lambert's cosine law.
		if(cosi <= 0)
		{
			continue;
		}

		float falloff = 1.0f / (tMax * tMax); /// inverse square law
		if(falloff < EPS)
		{
			continue;
		}

		lSamples += brdf * light.m_color * light.m_intensity * cosi * falloff * pdfEffect;
	}
	return lSamples / static_cast<float>(qlNumSamples);
}

float3 Renderer::CalcAllQuadLights(float3 p, float3 n, float3 brdf, uint pixelIndex)
{
	float3 l(0);
	int numQuadLights = static_cast<int>(scene.m_quadLightList.size());
	for(int i = 0; i < numQuadLights; ++i)
	{
		l += CalcQuadLight(scene.m_quadLightList[i], p, n, brdf, pixelIndex);
	}
	return l;
}
