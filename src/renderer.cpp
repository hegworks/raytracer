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

	scene = Scene();
	scene.LoadModels();
	PointLight& pl = scene.CreatePointLight();
	pl.m_pos.y = 1.0f;
	pl.m_intensity = 10.0f;

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
			accumulator[pixelIndex] += float4(Trace(r, pixelIndex, 0, tddIsPixelX, tddIsPixelY), 0);
			float4 avg = accumulator[pixelIndex] * scale;
			if(tdd && tddBBG || tdd && screen->pixels[pixelIndex] != 0x0) continue;
			screen->pixels[pixelIndex] = RGBF32_to_RGB8(&avg);
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
	if(camera.HandleInput(deltaTime) || (useACMMax && acmCounter > acmMax))
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

	scene.m_bvhs[0].Intersect(ray);
	if(ray.hit.t > BVH_FAR)
	{
		return 0; // or a fancy sky color
	}

	float3 rayDN = normalize(ray.D);
	// return O + t * D
	float3 p = ray.O + ray.hit.t * ray.D; /// intersection point
	//float3 n = scene.m_models[0].m_meshes[0].m_triangles[ray.hit.prim * 3]; /// normal of the intersection point
	float3 n0 = scene.m_models[0].m_meshes[0].m_normals[ray.hit.prim * 3]; /// normal of the intersection point
	float3 n1 = scene.m_models[0].m_meshes[0].m_normals[ray.hit.prim * 3 + 1]; /// normal of the intersection point
	float3 n2 = scene.m_models[0].m_meshes[0].m_normals[ray.hit.prim * 3 + 2]; /// normal of the intersection point

	float u = ray.hit.u;
	float v = ray.hit.v;
	float w = 1.0f - u - v;
	float3 n = float3((w * n0) + (u * n1) + (v * n2));

	bool hasHit = ray.hit.t < BVH_FAR;

	bool tddIsCameraY = tdd && IsCloseF(p.y, camera.camPos.y);
	TDDP(ray, p, n, screen, depth, tddIsPixelX, tddIsPixelY, tddIsCameraY);

	float3 l(0);
	Material mat = Material();
	switch(mat.m_type)
	{
		case Material::Type::DIFFUSE:
		{
			l += CalcLights(ray, p, n, pixelIndex, tddIsPixelX, tddIsPixelY, tddIsCameraY);
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
			l += mat.m_albedo * mat.m_glossiness * Trace(r, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			break;
		}
		case Material::Type::REFLECTIVE:
		{
			float3 rrdir = reflect(rayDN, n);
			Ray rr(p + rrdir * EPS, rrdir);
			l += mat.m_glossiness * Trace(rr, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			break;
		}
		case Material::Type::GLOSSY:
		{
			l += (1.0f - mat.m_glossiness) * CalcLights(ray, p, n, pixelIndex, tddIsPixelX, tddIsPixelY, tddIsCameraY);
			float3 rrdir = reflect(rayDN, n);
			Ray rr(p + rrdir * EPS, rrdir);
			l += mat.m_glossiness * Trace(rr, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			break;
		}
		case Material::Type::REFRACTIVE:
		{
			float fres;
			fresnel(rayDN, n, dbgIor, fres);

			float3 refracted(0);
			if((1.0f - fres) > EPS)
			{
				float3 refracDir = refract(rayDN, n, dbgIor);
				Ray refracR(p + refracDir * EPS, refracDir);
				//refracR.inside = !ray.inside; TODO
				refracted = Trace(refracR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			}

			float3 reflected(0);
			if(fres > EPS)
			{
				float3 reflecDir = reflect(rayDN, n);
				Ray reflecR(p + reflecDir * EPS, reflecDir);
				//reflecR.inside = ray.inside; TODO
				reflected = Trace(reflecR, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);
			}

			// here mat.m_glossiness is being used as density of the matter
			//float3 beer = ray.inside ? expf(-mat.m_albedo * mat.m_glossiness * ray.t) : 1.0f;
			//float3 alb = dbgBeer ? beer : mat.m_albedo;

			//l += alb * ((fres * reflected) + ((1.0f - fres) * refracted));
			l += ((fres * reflected) + ((1.0f - fres) * refracted));

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
			//return scene.GetAlbedo(ray.objIdx, ray.IntersectionPoint()); TODO
			return {0};
		case 3:
			return l;
		default:
			throw std::runtime_error("Unhandled situation");
	}
}

float3 Renderer::CalcLights(Ray& ray, float3 p, float3 n, uint pixelIndex, bool isTddPixelX, bool isTddPixelY, bool isTddCameraY)
{
	//float3 albedo = scene.GetAlbedo(ray.objIdx, p); /// albedo of the intersection point TODO
	float3 albedo = {0.5};
	//float3 wo = -ray.D; /// outgoing light direction
	float3 brdf = albedo / PI; // for diffuse (matte) surfaces

	float3 l(0); /// total outgoing radiance
	l += CalcPointLight(p, n, brdf, isTddPixelX, isTddPixelY, isTddCameraY);
	l += CalcSpotLight(p, n, brdf);
	l += CalcDirLight(p, n, brdf);
	l += CalcQuadLight(p, n, brdf, pixelIndex);

	return l;
}

float3 Renderer::CalcPointLight(float3 p, float3 n, float3 brdf, bool isTddPixelX, bool isTddPixelY, bool isTddCameraY)
{
	float3 l(0);
	int numPointLights = static_cast<int>(scene.m_pointLights.size());
	for(int i = 0; i < numPointLights; ++i)
	{
		PointLight& light = scene.m_pointLights[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
		float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

		Ray shadowRay(srPos, wi, tMax);
		bool isInShadow = scene.m_bvhs[0].IsOccluded(shadowRay); //TODO

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
	int numSpotLights = static_cast<int>(scene.m_spotLights.size());
	for(int i = 0; i < numSpotLights; ++i)
	{
		SpotLight& light = scene.m_spotLights[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
		float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

		Ray shadowRay(srPos, wi, tMax);
		bool isInShadow = scene.m_bvhs[0].IsOccluded(shadowRay); //TODO
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
	int numDirLights = static_cast<int>(scene.m_dirLights.size());
	for(int i = 0; i < numDirLights; ++i)
	{
		DirLight& light = scene.m_dirLights[i];
		float3 wi = -light.m_dir; /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)

		Ray shadowRay(srPos, wi);
		bool isInShadow = scene.m_bvhs[0].IsOccluded(shadowRay); //TODO
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
	int numQuadLights = static_cast<int>(scene.m_quadLights.size());
	for(int i = 0; i < numQuadLights; ++i)
	{
		QuadLight& light = scene.m_quadLights[i];
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
			bool isInShadow = scene.m_bvhs[0].IsOccluded(shadowRay); //TODO
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
