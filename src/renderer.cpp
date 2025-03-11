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

	for(int y = 0; y < SCRHEIGHT; y++)
	{
		for(int x = 0; x < SCRWIDTH; x++)
		{
			pixelSeeds[x + y * SCRWIDTH] = threadRng.InitSeed(1 + x + y * SCRWIDTH);
		}
	}

	acmCounter = 1;

	//PointLight& pl = scene.CreatePointLight();
	//pl.m_pos = float3(1, 15, 5);
	//pl.m_intensity = 800;

	DirLight& dl = scene.CreateDirLight();
	dl.m_intensity = 2.0f;

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
			const float xOffset = useAA ? RandomFloat(pixelSeeds[pixelIndex]) : 0.0f;
			const float yOffset = useAA ? RandomFloat(pixelSeeds[pixelIndex]) : 0.0f;
			Ray r = camera.GetPrimaryRay(static_cast<float>(x) + xOffset, static_cast<float>(y) + yOffset);
			float3 traced = Trace(r, pixelIndex, 0, tddIsPixelX, tddIsPixelY);
			if(dot(traced, traced) > dbgFF * dbgFF) traced = dbgFF * normalize(traced); // firefly suppressor
			accumulator[pixelIndex] += float4(traced, 0);
			float4 avg = accumulator[pixelIndex] * scale;
			if(tdd && tddBBG || tdd && screen->pixels[pixelIndex] != 0x0) continue;
			float4 gammaCorrected = float4(pow(avg.x, 1.0f / dbgGC), pow(avg.y, 1.0f / dbgGC), pow(avg.z, 1.0f / dbgGC), 1);
			screen->pixels[pixelIndex] = RGBF32_to_RGB8(&gammaCorrected);
			if(isDbgFixSeed) pixelSeeds[pixelIndex] = lastPixelSeeds[pixelIndex];
		}
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
	float3 n = scene.GetNormal(ray);
	bool inside = dot(ray.D, n) > 0.0f;
	if(inside) n = -n;

	bool tddIsCameraY = tdd && IsCloseF(p.y, camera.camPos.y);
	TDDP(ray, p, n, screen, depth, tddIsPixelX, tddIsPixelY, tddIsCameraY);

	float3 l(0);
	Material mat = scene.GetMaterial(ray);
	switch(mat.m_type)
	{
		case Material::Type::DIFFUSE:
		{
			l += CalcLights(ray, p, n, mat, pixelIndex, tddIsPixelX, tddIsPixelY, tddIsCameraY);
			break;
		}
		case Material::Type::DIFFUSE_PT:
		{
			float3 randPoint(0);
			while(true)
			{
				randPoint =
				{
					threadRng.RandomFloat(pixelSeeds[pixelIndex],-1.0f,1.0f),
					threadRng.RandomFloat(pixelSeeds[pixelIndex],-1.0f,1.0f),
					threadRng.RandomFloat(pixelSeeds[pixelIndex],-1.0f,1.0f),
				};
				float len = length(randPoint);
				if(len > 1e-160 && len <= 1.0f)
				{
					break;
				}
			}
			bool isInCorrectHemisphere = dot(randPoint, n) > 0.0f;
			if(!isInCorrectHemisphere) randPoint = -randPoint;
			float3 randDir = normalize(n + randPoint);
			Ray r(p + randDir * EPS, randDir);
			float3 contribution = mat.m_albedo * mat.m_factor0 * Trace(r, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			l += contribution;
			break;
		}
		case Material::Type::REFLECTIVE:
		{
			float3 rrdir = reflect(ray.D, n);
			Ray rr(p + rrdir * EPS, rrdir);
			l += mat.m_factor0 * Trace(rr, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			break;
		}
		case Material::Type::GLOSSY:
		{
			l += (1.0f - mat.m_factor0) * CalcLights(ray, p, n, mat, pixelIndex, tddIsPixelX, tddIsPixelY, tddIsCameraY);
			float3 rrdir = reflect(ray.D, n);
			Ray rr(p + rrdir * EPS, rrdir);
			l += mat.m_factor0 * Trace(rr, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			break;
		}
		case Material::Type::REFRACTIVE:
		{
			float3 localN = n;
			if(inside) localN = -n;
			float fres;
			fresnel(ray.D, localN, mat.m_factor1, fres);

			float3 refracted(0);
			if((1.0f - fres) > EPS)
			{
				float3 refracDir = refract(ray.D, localN, mat.m_factor1);
				Ray refracR(p + refracDir * EPS, refracDir);
				refracR.dummy1 = refracR.dummy1 == 0 ? 1 : 0;
				refracted = Trace(refracR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			}

			float3 reflected(0);
			if(fres > EPS)
			{
				float3 reflecDir = reflect(ray.D, localN);
				Ray reflecR(p + reflecDir * EPS, reflecDir);
				reflecR.dummy1 = ray.dummy1;
				reflected = Trace(reflecR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			}

			// here mat.m_factor0 is being used as density of the matter
			float3 beer = inside ? expf(-mat.m_albedo * mat.m_factor0 * ray.hit.t) : 1.0f;
			float3 alb = dbgBeer ? beer : mat.m_albedo;

			l += alb * ((fres * reflected) + ((1.0f - fres) * refracted));

			break;
		}
	}

	switch(ndal)
	{
		case 0:
			return hasHit ? (n + 1) * 0.5f : 0;
		case 1:
			return float3(ray.hit.t) * 0.1f;
		case 2:
			return scene.GetMaterial(ray).m_albedo;
		case 3:
			return l;
		default:
			throw std::runtime_error("Unhandled situation");
	}
}

float3 Renderer::CalcLights([[maybe_unused]] Ray& ray, float3 p, float3 n, const Material& mat, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, bool isTddCameraY)
{
	float3 albedo = mat.m_albedo;
	float3 brdf = albedo / PI; // for diffuse (matte) surfaces

	float3 l(0); /// total outgoing radiance
	l += CalcPointLight(p, n, brdf, isTddPixelX, isTddPixelY, isTddCameraY);
	l += CalcSpotLight(p, n, brdf);
	l += CalcDirLight(p, n, brdf);
	l += CalcQuadLight(p, n, brdf, pixelIndex);

	return l;
}

float3 Renderer::CalcPointLight(float3 p, float3 n, float3 brdf, bool isTddPixelX, bool isTddPixelY, [[maybe_unused]] bool isTddCameraY)
{
	float3 l(0);
	int numPointLights = static_cast<int>(scene.m_pointLightList.size());
	for(int i = 0; i < numPointLights; ++i)
	{
		PointLight& light = scene.m_pointLightList[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
		float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

		Ray shadowRay(srPos, wi, tMax);
		bool isInShadow = scene.IsOccluded(shadowRay);

		if(isTddPixelX && isTddPixelY)
		{
			// light pos
			if(tddPLP)
			{
				int2 o = WTS(lPos); /// origin
				screen->Box(o.x - 2, o.y - 2, o.x + 2, o.y + 2, 0x00ff00);
			}

			// shadow ray
			if(tddPLR)
			{
				float2 o = WTS(srPos); /// origin
				float2 d = WTS(lPos); /// destination

				uint color = isInShadow ? 0xff00ff : 0xffff00;
				if(isTddPixelY)
				{
					screen->Line(o.x, o.y, d.x, d.y, color);
				}
			}
		}

		if(isInShadow)
		{
			continue;
		}

		float cosi = dot(n, wi); /// Lambert's cosine law
		if(cosi <= 0)
		{
			continue;
		}

		float falloff = 1.0f / (tMax * tMax); /// inverse square law
		if(falloff < EPS)
		{
			continue;
		}

		l += brdf * light.m_color * light.m_intensity * cosi * falloff;
	}
	return l;
}

float3 Renderer::CalcSpotLight(float3 p, float3 n, float3 brdf)
{
	float3 l(0);
	int numSpotLights = static_cast<int>(scene.m_spotLightList.size());
	for(int i = 0; i < numSpotLights; ++i)
	{
		SpotLight& light = scene.m_spotLightList[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
		float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

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

		float cutoff = clamp((-dot(wi, light.m_dir) - light.m_cosO) / (light.m_cosI - light.m_cosO), 0.0f, 1.0f); // lerp. NOTICE the minus before dot
		if(cutoff <= 0)
		{
			continue;
		}

		l += brdf * light.m_color * light.m_intensity * cosi * falloff * cutoff * cutoff;
	}
	return l;
}

float3 Renderer::CalcDirLight(float3 p, float3 n, float3 brdf)
{
	float3 l(0);
	int numDirLights = static_cast<int>(scene.m_dirLightList.size());
	for(int i = 0; i < numDirLights; ++i)
	{
		DirLight& light = scene.m_dirLightList[i];
		float3 wi = -light.m_dir; /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)

		Ray shadowRay(srPos, wi);
		bool isInShadow = scene.IsOccluded(shadowRay);
		if(isInShadow)
		{
			continue;
		}

		float cosi = dot(n, wi); /// Lambert's cosine law
		if(cosi <= 0)
		{
			continue;
		}

		l += brdf * light.m_color * light.m_intensity * cosi;
	}
	return l;
}

float3 Renderer::CalcQuadLight(float3 p, float3 n, float3 brdf, uint pixelIndex)
{
	float3 l(0);
	int numQuadLights = static_cast<int>(scene.m_quadLightList.size());
	for(int i = 0; i < numQuadLights; ++i)
	{
		QuadLight& light = scene.m_quadLightList[i];
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
		l += lSamples / static_cast<float>(qlNumSamples);
	}
	return l;
}
