#include "precomp.h"

#include <common.h>

#include "DBG.h"
#include "ImGuiThemeManager.h"
#include "Material.h"
#include "Model.h"

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	// create fp32 rgb pixel buffer to render to
	accumulator = static_cast<float4*>(MALLOC64((size_t)SCRSIZE * 16));
	memset(accumulator, 0, static_cast<size_t>(SCRSIZE) * 16);

	illuminations = static_cast<float4*>(MALLOC64((size_t)SCRSIZE * 16));
	memset(illuminations, 0, static_cast<size_t>(SCRSIZE) * 16);

	for(int y = 0; y < SCRHEIGHT; y++)
	{
		for(int x = 0; x < SCRWIDTH; x++)
		{
			pixelSeeds[x + y * SCRWIDTH] = threadRng.InitSeed(1 + x + y * SCRWIDTH);
		}
	}

	acmCounter = 1;

#ifdef _GAME
	ImGuiThemeManager::SetNewDarkTheme();
	m_gameManager.Init(&scene, this);
#elif defined(_ENINGE)
	ImGuiThemeManager::SetDarkThemeColors();
#endif

	/*QuadLight& ql = scene.CreateQuadLight();
	ql.m_quad.size = 5;
	ql.m_intensity = 5;
	ql.m_quad.m_pos = {0,5,0};
	ql.m_quad.T = mat4::Translate(ql.m_quad.m_pos);
	ql.m_quad.invT = ql.m_quad.T.FastInvertedTransformNoScale();*/

#ifdef _DEBUG
	//dbgScrRangeX = {(SCRWIDTH / 2) - 150,(SCRWIDTH / 2) + 150};
	//dbgScrRangeY = {(SCRHEIGHT / 2) - 150,(SCRHEIGHT / 2) + 150};
#endif // _DEBUG
}

#ifdef _ENGINE
// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(const float deltaTime)
{
	// pixel loop
	const Timer t;
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
		for(int i = 0; i < SCRSIZE; ++i)
		{
			sum += (illuminations[i].x + illuminations[i].y + illuminations[i].z) / 3.0f;
		}
		sum /= 1000;
	}

	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if(alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRSIZE) / (avg * 1000);
	dfps = fps, drps = rps, davg = avg;
	//printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps);

	const bool cameraChanged = camera.HandleInput(deltaTime);
	const bool objectRotationChanged = HandleKeyboardRotations(deltaTime);
	if(cameraChanged || objectRotationChanged || resetAccumulator || !useACM)
	{
		memset(accumulator, 0, SCRSIZE * 16);
		acmCounter = 1;
		resetAccumulator = false;
	}

	if(tdd)
	{
		screen->Line(0, 360, SCRWIDTH - 1, 360, 0xff0000);
	}
}
#endif

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
	const bool hasHit = ray.hit.t < BVH_FAR;
	if(!hasHit)
	{
		if(useSD)
			return scene.SampleSky(ray);
		return 0;
	}

	const float3 p = ray.O + ray.hit.t * ray.D; /// intersection point
	float3 n = scene.CalcSmoothNormal(ray);
	const bool inside = dot(ray.D, scene.CalcRawNormal(ray)) > 0.0f;
	if(inside) n = -n;

#ifdef _ENGINE
	bool tddIsCameraY = tdd && IsCloseF(p.y, camera.camPos.y);
	TDDP(ray, p, n, screen, depth, tddIsPixelX, tddIsPixelY, tddIsCameraY);
#endif

	float3 l(0);
	const Model& model = scene.GetModel(ray);
	const Material mat = scene.GetMaterial(ray);
	const float3 albedo = mat.m_albedo * Scene::SampleTexture(ray, model);
	const float3 brdf = albedo / PI; // for diffuse (matte) surfaces
	switch(mat.m_type)
	{
		case Material::Type::DIFFUSE:
		{
			l += CalcLights(ray, p, n, brdf, pixelIndex);
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
			const float density = mat.m_factor0;
			const float ior = mat.m_factor1;
			float3 localN = n;
			if(inside) localN = -n;
			float fres;
			fresnel(ray.D, localN, ior, fres);
			const float oneMinusFres = 1.0f - fres;

			if(fres > EPS || oneMinusFres > EPS)
			{
				if(dbgSF)
				{
					const bool reflectTrueRefractFalse = threadRng.RandomFloat(pixelSeeds[pixelIndex]) < fres;
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
				finalMatColor = albedo;
			}

			Ray finalRay(p + finalDir * EPS, finalDir);

			float3 finalTrace = finalMatColor * Trace(finalRay, pixelIndex, depth + 1, tddIsPixelX, tddIsPixelY);

			float3 directLight = CalcLights(ray, p, n, brdf, pixelIndex);

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

#ifdef _ENGINE
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
#elif defined(_GAME)
	return l;
#endif
}

float3 Renderer::CalcLights([[maybe_unused]] Ray& ray, float3 p, float3 n, float3 brdf, uint pixelIndex)
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

float3 Renderer::CalcSpotLight(const SpotLight& light, const float3& p, const float3& n, const float3& brdf) const
{
	const float3 lPos = light.m_pos; /// LightPos
	const float3 vi = lPos - p; /// Light Vector
	const float3 wi = normalize(vi); /// incoming light direction
	const float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
	const float tMax = length(vi) - TWO_EPS; /// distance between srPos and lPos (Considering EPS)

	const float cosi = dot(n, wi); /// Lambert's cosine law.
	if(cosi <= 0)
	{
		return 0;
	}

	const float falloff = 1.0f / (tMax * tMax); /// inverse square law
	if(falloff < EPS)
	{
		return 0;
	}

	const Ray shadowRay(srPos, wi, tMax);
	const bool isInShadow = scene.IsOccluded(shadowRay);
	if(isInShadow)
	{
		return 0;
	}

	const float cutoff = clamp((-dot(wi, light.m_dir) - light.m_cosO) / (light.m_cosI - light.m_cosO), 0.0f, 1.0f); // lerp. NOTICE the minus before dot
	if(cutoff <= 0)
	{
		return 0;
	}

	return brdf * light.m_color * light.m_intensity * cosi * falloff * cutoff * cutoff;
}

float3 Renderer::CalcAllSpotLights(const float3& p, const float3& n, const float3& brdf) const
{
	float3 l(0);
	const int numSpotLights = static_cast<int>(scene.m_spotLightList.size());
	for(int i = 0; i < numSpotLights; ++i)
	{
		l += CalcSpotLight(scene.m_spotLightList[i], p, n, brdf);
	}
	return l;
}

float3 Renderer::CalclDirLight(const DirLight& light, const float3& p, const float3& n, const float3& brdf) const
{
	const float3 wi = -light.m_dir; /// incoming light direction
	const float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)

	const float cosi = dot(n, wi); /// Lambert's cosine law
	if(cosi <= 0)
	{
		return 0;
	}

	const Ray shadowRay(srPos, wi);
	const bool isInShadow = scene.IsOccluded(shadowRay);
	if(isInShadow)
	{
		return 0;
	}

	return brdf * light.m_color * light.m_intensity * cosi;
}

float3 Renderer::CalcAllDirLights(const float3& p, const float3& n, const float3& brdf) const
{
	float3 l(0);
	const int numDirLights = static_cast<int>(scene.m_dirLightList.size());
	for(int i = 0; i < numDirLights; ++i)
	{
		l += CalclDirLight(scene.m_dirLightList[i], p, n, brdf);
	}
	return l;
}

float3 Renderer::CalcQuadLight(const QuadLight& light, const float3& p, const float3& n, const float3& brdf, const uint pixelIndex)
{
	float3 lSamples = float3(0);
	const float3 lightDir = -light.m_quad.GetNormal();
	const float pdfEffect = 1.0f / light.GetArea();

	for(int j = 0; j < qlNumSamples; ++j)
	{
		const float3 a = light.GetRandomPoint(pixelSeeds[pixelIndex]);
		const float3 vi = a - p;
		const float3 wi = normalize(vi);
		const float tMax = length(vi) - TWO_EPS;

		if(qlOneSided)
		{
			const bool isOppositeSide = dot(lightDir, wi) <= 0;
			if(isOppositeSide)
			{
				continue;
			}
		}

		const float cosi = dot(n, wi); /// Lambert's cosine law.
		if(cosi <= 0)
		{
			continue;
		}

		const float falloff = 1.0f / (tMax * tMax); /// inverse square law
		if(falloff < EPS)
		{
			continue;
		}

		const float3 srPos = p + wi * EPS;
		const Ray shadowRay(srPos, wi, tMax);
		const bool isInShadow = scene.IsOccluded(shadowRay);
		if(isInShadow)
		{
			continue;
		}

		lSamples += brdf * light.m_color * light.m_intensity * cosi * falloff * pdfEffect;
	}
	return lSamples / static_cast<float>(qlNumSamples);
}

float3 Renderer::CalcAllQuadLights(const float3& p, const float3& n, const float3& brdf, const uint pixelIndex)
{
	float3 l(0);
	const int numQuadLights = static_cast<int>(scene.m_quadLightList.size());
	for(int i = 0; i < numQuadLights; ++i)
	{
		l += CalcQuadLight(scene.m_quadLightList[i], p, n, brdf, pixelIndex);
	}
	return l;
}

bool Renderer::HandleKeyboardRotations(const float deltaTime)
{
	bool rotChanged = false;

	const float speed = 0.001f * deltaTime;
	if(IsKeyDown(GLFW_KEY_U)) RotateAroundWorldAxis(scene.m_tranformList[selectedInstIdx], float3(1, 0, 0), speed), rotChanged = true;
	if(IsKeyDown(GLFW_KEY_I)) RotateAroundWorldAxis(scene.m_tranformList[selectedInstIdx], float3(0, 1, 0), speed), rotChanged = true;
	if(IsKeyDown(GLFW_KEY_O)) RotateAroundWorldAxis(scene.m_tranformList[selectedInstIdx], float3(0, 0, 1), speed), rotChanged = true;

	if(IsKeyDown(GLFW_KEY_J)) RotateAroundWorldAxis(scene.m_tranformList[selectedInstIdx], float3(1, 0, 0), -speed), rotChanged = true;
	if(IsKeyDown(GLFW_KEY_K)) RotateAroundWorldAxis(scene.m_tranformList[selectedInstIdx], float3(0, 1, 0), -speed), rotChanged = true;
	if(IsKeyDown(GLFW_KEY_L)) RotateAroundWorldAxis(scene.m_tranformList[selectedInstIdx], float3(0, 0, 1), -speed), rotChanged = true;

	if(rotChanged)
	{
		Scene::SetBlasTransform(scene.m_blasList[selectedInstIdx], scene.m_tranformList[selectedInstIdx]);
		scene.BuildTlas();
	}

	return rotChanged;
}

void Renderer::MouseMove(int x, int y)
{
	mousePos = {x,y};
	windowCoord = {x,y};
	windowCoordF = windowCoord;
	screenCoordF = windowCoordF * INV_SCRSCALE;
	screenCoord = {static_cast<int>(screenCoordF.x),static_cast<int>(screenCoordF.y)};

#ifdef _GAME
	m_gameManager.OnMouseMove(windowCoordF, windowCoord, screenCoordF, screenCoord);
#endif
}

void Renderer::MouseUp(const int button)
{
#ifdef _GAME
	m_gameManager.OnMouseUp(button);
#endif
}

void Renderer::MouseDown(int button)
{
#ifdef _GAME
	m_gameManager.OnMouseDown(button);
#endif

	if(isDbgPixel && !isDbgPixelClicked)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			isDbgPixelClicked = true;
		}
	}
	else
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT && !ImGui::GetIO().WantCaptureMouse)
		{
			selectedInstIdx = hoveredInst;
			selectedMeshIdx = hoveredMesh;
		}
	}
}

void Renderer::KeyDown(const int key)
{
#ifdef _GAME
	m_gameManager.OnKeyDown(key);
#endif

#ifdef _ENGINE
	static int theme = 0;
	if(key == GLFW_KEY_T)
	{
		theme = (theme + 1) % 9;
		printf("ThemeNumber: %i\n", theme);
		if(theme == 0) ImGuiThemeManager::SetBessDarkThemeColors();
		if(theme == 1) ImGuiThemeManager::SetCatpuccinMochaColors();
		if(theme == 2) ImGuiThemeManager::SetDarkThemeColors();
		if(theme == 3) ImGuiThemeManager::SetFluentUiLightTheme();
		if(theme == 4) ImGuiThemeManager::SetFluentUiTheme();
		if(theme == 5) ImGuiThemeManager::SetGlassTheme();
		if(theme == 6) ImGuiThemeManager::SetMaterialYouColors();
		if(theme == 7) ImGuiThemeManager::SetModernColors();
		if(theme == 8) ImGuiThemeManager::SetNewDarkTheme();
	}
#endif

	if(key == GLFW_KEY_Q)
	{
		resetAccumulator = true;
	}

	if(isDbgPixel && isDbgPixelClicked)
	{
		if(key == GLFW_KEY_UP) dbgpixel.y = dbgpixel.y == 0 ? 0 : dbgpixel.y - 1;
		if(key == GLFW_KEY_DOWN) dbgpixel.y = dbgpixel.y == SCRHEIGHT - 1 ? SCRHEIGHT - 1 : dbgpixel.y + 1;
		if(key == GLFW_KEY_LEFT) dbgpixel.x = dbgpixel.x == 0 ? 0 : dbgpixel.x - 1;
		if(key == GLFW_KEY_RIGHT) dbgpixel.x = dbgpixel.x == SCRWIDTH - 1 ? SCRWIDTH - 1 : dbgpixel.x + 1;

		if(key == GLFW_KEY_ENTER)
		{
			dbgpixel =
			{
				static_cast<int>(static_cast<float>(dbgpixel.x) * INV_SCRSCALE),
				static_cast<int>(static_cast<float>(dbgpixel.y) * INV_SCRSCALE)
			};
			printf("DBG PIXEL AT %i,%i\n", dbgpixel.x, dbgpixel.y);
			isDbgPixelEntered = true;
		}
	}
}

void Renderer::RotateAroundWorldAxis(Transform& transform, const float3& worldAxis, const float angleRadians)
{
	const quat worldRotation = quat::FromAxisAngle(worldAxis, angleRadians);
	transform.m_rot = worldRotation * transform.m_rot;
#if 0
	transform.m_rotAngles += worldAxis * RAD_TO_DEG(angleRadians);
#elif 1
	transform.m_rotAngles = RAD_TO_DEG(transform.m_rot.toEuler());
#elif 0
	float3 eu = 0;
	quat::DecomposeQuaternionToEuler(transform.m_rot, eu);
	transform.m_rotAngles = eu;
#endif
}
