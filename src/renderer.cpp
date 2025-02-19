#include "precomp.h"

#include <common.h>

#include "DBG.h"
#include "UknittyMath.h"

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
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace(Ray& ray, int x, int y)
{
	scene.FindNearest(ray);
	if(ray.objIdx == -1)
	{
		return 0; // or a fancy sky color
	}

	float3 p = ray.IntersectionPoint(); /// intersection point
	float3 n = scene.GetNormal(ray.objIdx, p, ray.D); /// normal of the intersection point
	float3 albedo = scene.GetAlbedo(ray.objIdx, p); /// albedo of the intersection point
	float3 wo = -ray.D; /// outgoing light direction
	float3 brdf = albedo / PI; // for diffuse (matte) surfaces

	const bool canTDD = CanTDD(ray.objIdx, p);
	if(canTDD)
	{
		int2 pd = WTS(p); /// intersection point debug
		screen->Plot(pd.x, pd.y, 0xffffff);

		if(pd.x % tddrx == 0)
		{
			// primary ray
			if(tddPRay)
			{
				int2 o = WTS(ray.O);
				int2 d = pd;
				screen->Line(o.x, o.y, d.x, d.y, 0xff0000);
			}

			// normal
			if(tddPN)
			{
				int2 o = pd;
				int2 d = WTS(p + n / 2.0f);
				screen->Line(o.x, o.y, d.x, d.y, 0x00ff00);
			}

			// normal length
			if(tddPNL)
			{
				int2 o = WTS(p + n / 2.0f); /// normal debug point

				char t[20];
				sprintf(t, "%.2f", length(n));
				if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0x00ff00);
			}

			// ray length
			if(tddPRayL)
			{
				int2 o = {pd.x, pd.y - 5};

				char t[20];
				sprintf(t, "%.2f", ray.t);
				if(DBGCanPrint(o)) screen->Print(t, o.x, o.y, 0xff0000);
			}
		}
	}

	float3 l(0.0f); /// total outgoing radiance

	for(int i = 0; i < static_cast<int>(scene.m_pointLights.size()); ++i)
	{
		PointLight& light = scene.m_pointLights[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + wi * EPS; /// ShadowRayPos (considering EPS)
		float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

		Ray shadowRay(srPos, wi, tMax);
		bool isInShadow = scene.IsOccluded(shadowRay);

		if(canTDD)
		{
			// light pos
			{
				int2 o = WTS(lPos); /// origin
				screen->Box(o.x - 2, o.y - 2, o.x + 2, o.y + 2, 0x00ff00);
			}

			// shadow ray
			{
				int2 o = WTS(srPos); /// origin
				int2 d = WTS(lPos); /// destination

				int2 pd = WTS(p);
				uint color = isInShadow ? 0xff00ff : 0xffff00;
				if(pd.x % tddrx == 0)
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

	for(int i = 0; i < static_cast<int>(scene.m_spotLights.size()); ++i)
	{
		SpotLight& light = scene.m_spotLights[i];
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

	for(int i = 0; i < static_cast<int>(scene.m_dirLights.size()); ++i)
	{
		DirLight& light = scene.m_dirLights[i];
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

	for(int i = 0; i < static_cast<int>(scene.m_quadLights.size()); ++i)
	{
		QuadLight& light = scene.m_quadLights[i];
		float3 lSamples = float3(0);
		float3 lightDir = -light.m_quad.GetNormal();
		float pdfEffect = 1 / light.GetPDF();

		for(int j = 0; j < qlNumSamples; ++j)
		{
			float3 a = light.GetRandomPoint(pixelSeeds[x + y * SCRWIDTH]);
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

	if(nda == 0)
	{
		return (n + 1) * 0.5f;
	}
	else if(nda == 1)
	{
		return float3(ray.t) * 0.1f;
	}
	else if(nda == 2)
	{
		return albedo;
	}
	else if(nda == 3)
	{
		return l;
	}

	throw std::runtime_error("Unhandled situation");
}

// -----------------------------------------------------------
// Main application tick function - Executed once per frame
// -----------------------------------------------------------
void Renderer::Tick(float deltaTime)
{
	// animation
	if(animating) scene.SetTime(anim_time += deltaTime * 0.002f);
	// pixel loop
	Timer t;
	const float scale = 1.0f / static_cast<float>(acmCounter++);

	if(tdd) screen->Clear(0);

	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for(int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for(int x = 0; x < SCRWIDTH; x++)
		{
			const float xOffset = useAA ? RandomFloat(pixelSeeds[x + y * SCRWIDTH]) : 0.0f;
			const float yOffset = useAA ? RandomFloat(pixelSeeds[x + y * SCRWIDTH]) : 0.0f;
			Ray r = camera.GetPrimaryRay(static_cast<float>(x) + xOffset, static_cast<float>(y) + yOffset);
			accumulator[x + y * SCRWIDTH] += float4(Trace(r, x, y), 0);
			float4 avg = accumulator[x + y * SCRWIDTH] * scale;
			if(!tdd || screen->pixels[x + y * SCRWIDTH] == 0x0) // if pixel has not been colored by tdd
			{
				screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(&avg);
			}
		}
	}
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if(alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / (avg * 1000);
	dfps = fps, drps = rps, davg = avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps);
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
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	if(tdd)
	{
		ImGui::Begin("2D Debugger");

		ImGui::DragFloat("SceneScale", &tddSceneScale, 0.01f, -5, 5);
		ImGui::DragInt2("Offset", &tddOffset.x);
		ImGui::DragInt("rx", &tddrx, 0.5f, 1, 200);

		ImGui::Checkbox("Primary Ray", &tddPRay);
		ImGui::Checkbox("Primary Ray Length", &tddPRayL);
		ImGui::Checkbox("P Normal", &tddPN);
		ImGui::Checkbox("P Normal Length", &tddPNL);

		ImGui::End();
	}

	// animation toggle
	ImGui::Text("avg	fps	rps");
	ImGui::Text("%.1f	%.0f	%.0f", davg, dfps, drps);
	ImGui::SameLine();
	if(ImGui::Button("ResetCam"))
	{
		tddResetCam = true;
	}

	ImGui::Checkbox("Animate", &animating);
	ImGui::SameLine();
	ImGui::Checkbox("AA", &useAA);
	ImGui::SameLine();
	ImGui::Checkbox("TDD", &tdd);

	// ray query on mouse
	Ray r = camera.GetPrimaryRay((float)mousePos.x, (float)mousePos.y);
	scene.FindNearest(r);
	bool isInScreen = mousePos.x >= 0 && mousePos.x < SCRWIDTH && mousePos.y >= 0 && mousePos.y < SCRHEIGHT;
	uint pixel = 0xFF00FF, red = -1, green = -1, blue = -1;
	if(isInScreen)
	{
		pixel = screen->pixels[mousePos.x + mousePos.y * SCRWIDTH];
		red = (pixel & 0xFF0000) >> 16;
		green = (pixel & 0x00FF00) >> 8;
		blue = pixel & 0x0000FF;
	}
	ImVec4 color = isInScreen ? ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f) : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	ImGui::ColorButton("", color);
	ImGui::SameLine();

	ImGui::Text("%u,%u,%u  %i,%i  %i", red, green, blue, mousePos.x, mousePos.y, r.objIdx);

	ImGui::SliderInt("ndal", &nda, 0, 3);

	ImGui::Checkbox("ACM MAX", &useACMMax);
	ImGui::SameLine();
	ImGui::SliderInt(" ", &acmMax, 1, 1000);

	// credits to Okke for the idea of creating lights at runtime
	if(ImGui::Button("+ PointL"))
	{
		scene.CreatePointLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ SpotL"))
	{
		scene.CreateSpotLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ DirL"))
	{
		scene.CreateDirLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ QuadL"))
	{
		scene.CreateQuadLight();
	}

	if(!scene.m_pointLights.empty())
	{
		if(ImGui::CollapsingHeader("PointLights"))
		{
			for(int i = 0; i < static_cast<int>(scene.m_pointLights.size()); i++)
			{
				if(ImGui::TreeNode(("PL " + std::to_string(i)).c_str()))
				{
					ImGui::DragFloat3("Pos", &scene.m_pointLights[i].m_pos.x, 0.01f);
					ImGui::ColorEdit3("Color", &scene.m_pointLights[i].m_color.x);
					ImGui::DragFloat("Intensity", &scene.m_pointLights[i].m_intensity, 0.01f, 0.0f, 1000.0f);

					ImGui::TreePop();
				}
			}
		}
	}
	if(!scene.m_spotLights.empty())
	{
		if(ImGui::CollapsingHeader("SpotLights"))
		{
			for(int i = 0; i < static_cast<int>(scene.m_spotLights.size()); i++)
			{
				if(ImGui::TreeNode(("SL " + std::to_string(i)).c_str()))
				{
					ImGui::DragFloat3("Pos", &scene.m_spotLights[i].m_pos.x, 0.01f);
					ImGui::ColorEdit3("Color", &scene.m_spotLights[i].m_color.x);
					ImGui::DragFloat("Intensity", &scene.m_spotLights[i].m_intensity, 0.01f, 0.0f, 1000.0f);
					float3 dir = scene.m_spotLights[i].m_dir;
					ImGui::DragFloat3("Dir", &dir.x, 0.001f, -1.0f, 1.0f);
					scene.m_spotLights[i].m_dir = normalize(dir);

					ImGui::DragFloat("CosI", &scene.m_spotLights[i].m_cosI, 0.001f, scene.m_spotLights[i].m_cosO, 1.0f);
					ImGui::SameLine();
					ImGui::Text("%.2f", RAD_TO_DEG(acos(scene.m_spotLights[i].m_cosI)));

					ImGui::DragFloat("CosO", &scene.m_spotLights[i].m_cosO, 0.001f, 0.0f, scene.m_spotLights[i].m_cosI);
					ImGui::SameLine();
					ImGui::Text("%.2f", RAD_TO_DEG(acos(scene.m_spotLights[i].m_cosO)));

					ImGui::TreePop();
				}
			}
		}
	}
	if(!scene.m_dirLights.empty())
	{
		if(ImGui::CollapsingHeader("DirLights"))
		{
			for(int i = 0; i < static_cast<int>(scene.m_dirLights.size()); i++)
			{
				if(ImGui::TreeNode(("DL " + std::to_string(i)).c_str()))
				{
					DirLight& light = scene.m_dirLights[i];
					float3 dir = light.m_dir;
					ImGui::DragFloat3("Dir", &dir.x, 0.01f);
					light.m_dir = normalize(dir);
					ImGui::ColorEdit3("Color", &light.m_color.x);
					ImGui::DragFloat("Intensity", &light.m_intensity, 0.01f, 0.0f, 1000.0f);

					ImGui::TreePop();
				}
			}
		}
	}
	if(!scene.m_quadLights.empty())
	{
		if(ImGui::CollapsingHeader("QuadLights"))
		{
			ImGui::SliderInt("Samples", &qlNumSamples, 1, 32);
			ImGui::Checkbox("1 Sided", &qlOneSided);

			for(int i = 0; i < static_cast<int>(scene.m_quadLights.size()); i++)
			{
				if(ImGui::TreeNode(("QL " + std::to_string(i)).c_str()))
				{
					QuadLight& light = scene.m_quadLights[i];

					ImGui::DragFloat3("Pos", &light.m_quad.m_pos.x, 0.01f);
					ImGui::DragFloat3("Dir", &light.m_quad.m_dir.x, 1.0f);
					mat4 baseMat = mat4::Identity();
					baseMat = baseMat * mat4::RotateX(DEG_TO_RAD(light.m_quad.m_dir.x));
					baseMat = baseMat * mat4::RotateY(DEG_TO_RAD(light.m_quad.m_dir.y));
					baseMat = baseMat * mat4::RotateZ(DEG_TO_RAD(light.m_quad.m_dir.z));
					light.m_quad.T = mat4::Translate(light.m_quad.m_pos) * baseMat;
					light.m_quad.invT = light.m_quad.T.FastInvertedTransformNoScale();

					ImGui::DragFloat("Size", &light.m_quad.size, 0.01f, 0.0f, 100.0f);

					ImGui::ColorEdit3("Color", &light.m_color.x);
					ImGui::DragFloat("Intensity", &light.m_intensity, 0.01f, 0.0f, 1000.0f);

					ImGui::TreePop();
				}
			}
		}
	}

}

int2 Renderer::WTS(float3 p)
{
	int x = (int)floorf(range_to_range(-4 * tddSceneScale, 4 * tddSceneScale, 0, SCRWIDTH, p.x)) + tddOffset.x;
	int y = (int)floorf(range_to_range(2.25 * tddSceneScale, -2.25 * tddSceneScale, 0, SCRHEIGHT, p.z)) + tddOffset.y;
	return {x,y};
}

bool Renderer::CanTDD(int objIdx, float3 p)
{
	return tdd && IsCloseF(p.y, camera.camPos.y) && (objIdx == 1 || objIdx == 3 || objIdx == 10);
}
