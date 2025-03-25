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
	if(dbgSL) // Stochastically choosing a light
	{
		int numSamples = dbgSLS;
		float3 stochasticL(0);
		const uint numSpotLights = static_cast<uint>(scene.m_spotLightList.size());
		const uint numDirLights = static_cast<uint>(scene.m_dirLightList.size());
		const uint numQuadLights = static_cast<uint>(scene.m_quadLightList.size());
		const uint numPointLights = static_cast<uint>(scene.m_pointLightList.size());
		const uint numLights = numPointLights + numSpotLights + numDirLights + numQuadLights;

		if(numLights == 0) return 0;

		if(numPointLights > 0)
		{
			for(int i = 0; i < numSamples; ++i)
			{
				uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], 0, numLights);

				if(randIdx < numPointLights)
				{
					PointLight& light = scene.m_pointLightList[randIdx];
					stochasticL += CalcPointLight(light, p, n, brdf, isTddPixelX, isTddPixelY);
				}
			}
		}

		if(numSpotLights > 0)
		{
			for(int i = 0; i < numSamples; ++i)
			{
				uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], numPointLights, numPointLights + numSpotLights);

				if(randIdx < numPointLights + numSpotLights)
				{
					SpotLight& light = scene.m_spotLightList[randIdx - numPointLights];
					stochasticL += CalcSpotLight(light, p, n, brdf);
				}
			}
		}

		if(numDirLights > 0)
		{
			for(int i = 0; i < numSamples; ++i)
			{
				uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], numPointLights + numSpotLights, numPointLights + numSpotLights + numDirLights);

				if(randIdx < numPointLights + numSpotLights + numDirLights)
				{
					DirLight& light = scene.m_dirLightList[randIdx - numPointLights - numSpotLights];
					stochasticL += CalclDirLight(light, p, n, brdf);
				}
			}
		}

		if(numQuadLights > 0)
		{
			for(int i = 0; i < numSamples; ++i)
			{
				uint randIdx = threadRng.RandomUInt(pixelSeeds[pixelIndex], numPointLights + numSpotLights + numDirLights, numLights);

				if(randIdx < numLights)
				{
					QuadLight& light = scene.m_quadLightList[randIdx - numPointLights - numSpotLights - numDirLights];
					stochasticL += CalcQuadLight(light, p, n, brdf, pixelIndex);
				}
			}
		}

		return stochasticL * (float)numLights / (float)numSamples;

	}
	else // calclulating all the lights
	{
		float3 l(0); /// total outgoing radiance
		l += CalcAllPointLights(p, n, brdf, isTddPixelX, isTddPixelY, isTddCameraY);
		l += CalcAllSpotLights(p, n, brdf);
		l += CalcAllDirLights(p, n, brdf);
		l += CalcAllQuadLights(p, n, brdf, pixelIndex);

		return l;
	}
}


float3 Renderer::CalcPointLight(const PointLight& light, float3 p, float3 n, float3 brdf, bool isTddPixelX, bool isTddPixelY)
{
	// vi: incoming light vector
	// float3 vi = light.m_pos - p;
	float vix = light.x - p.x;
	float viy = light.y - p.y;
	float viz = light.z - p.z;

	// t: distance between shadowRayPos and lightPos
	// float t = length(vi);
	float t = sqrt(vix * vix + viy * viy + viz * viz);

	// wi: incoming light direction
	// float3 wi = vi / t;
	float wix = vix / t;
	float wiy = viy / t;
	float wiz = viz / t;

	// lambert's cosine law
	// float cosi = dot(n,wi)
	float cosi = n.x * wix + n.y * wiy + n.z * wiz;
	if(cosi <= 0)
		return 0;

	// shadow ray
	const float3 wi = {wix, wiy, wiz};
	Ray shadowRay(p + wi * EPS, wi, t - EPS * 2.0f);
	if(scene.IsOccluded(shadowRay))
		return 0;

	// inverse square law
	float falloff = 1.0f / (t * t);
	if(falloff < EPS)
		return 0;

	float r = brdf.x * light.r;
	float g = brdf.y * light.g;
	float b = brdf.z * light.b;

	return float3(r, g, b) * light.i * cosi * falloff;
}

float3 Renderer::CalcAllPointLights(float3 p, float3 n, float3 brdf, bool isTddPixelX, bool isTddPixelY, [[maybe_unused]] bool isTddCameraY)
{
	float3 l(0);
	int numPointLights = static_cast<int>(scene.m_pointLightList.size());
	for(int i = 0; i < numPointLights; ++i)
	{
		l += CalcPointLight(scene.m_pointLightList[i], p, n, brdf, isTddPixelX, isTddPixelY);
	}
	return l;
}

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
