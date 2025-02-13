#include "precomp.h"

#include <common.h>

// -----------------------------------------------------------
// Initialize the renderer
// -----------------------------------------------------------
void Renderer::Init()
{
	// create fp32 rgb pixel buffer to render to
	accumulator = (float4*)MALLOC64(SCRWIDTH * SCRHEIGHT * 16);
	memset(accumulator, 0, SCRWIDTH * SCRHEIGHT * 16);
}

// -----------------------------------------------------------
// Evaluate light transport
// -----------------------------------------------------------
float3 Renderer::Trace(Ray& ray)
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

	float3 l(0.0f); /// total outgoing radiance

	for(int i = 0; i < static_cast<int>(scene.m_pointLights.size()); ++i)
	{
		PointLight& light = scene.m_pointLights[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + n * EPS; /// ShadowRayPos (considering EPS)
		float tMax = length(vi) - EPS * 2; /// distance between srPos and lPos (Considering EPS)

		Ray shadowRay(srPos, wi, tMax);
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

		float falloff = 1 / tMax * tMax; /// inverse square law

		l += brdf * light.m_color * light.m_intensity * cosi * falloff;
	}

	for(int i = 0; i < static_cast<int>(scene.m_spotLights.size()); ++i)
	{
		SpotLight& light = scene.m_spotLights[i];
		float3 lPos = light.m_pos; /// LightPos
		float3 vi = lPos - p; /// Light Vector
		float3 wi = normalize(vi); /// incoming light direction
		float3 srPos = p + n * EPS; /// ShadowRayPos (considering EPS)
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

		float cutoff = clamp((-dot(wi, light.m_dir) - light.m_cosO) / (light.m_cosI - light.m_cosO), 0.0f, 1.0f); // lerp. NOTICE the minus before dot
		if(cutoff <= 0)
		{
			continue;
		}

		float falloff = 1 / tMax * tMax; /// inverse square law

		l += brdf * light.m_color * light.m_intensity * cosi * falloff * cutoff;
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
	// lines are executed as OpenMP parallel tasks (disabled in DEBUG)
#pragma omp parallel for schedule(dynamic)
	for(int y = 0; y < SCRHEIGHT; y++)
	{
		// trace a primary ray for each pixel on the line
		for(int x = 0; x < SCRWIDTH; x++)
		{
			float4 pixel = float4(Trace(camera.GetPrimaryRay((float)x, (float)y)), 0);
			// translate accumulator contents to rgb32 pixels
			screen->pixels[x + y * SCRWIDTH] = RGBF32_to_RGB8(&pixel);
			accumulator[x + y * SCRWIDTH] = pixel;
		}
	}
	// performance report - running average - ms, MRays/s
	static float avg = 10, alpha = 1;
	avg = (1 - alpha) * avg + alpha * t.elapsed() * 1000;
	if(alpha > 0.05f) alpha *= 0.5f;
	float fps = 1000.0f / avg, rps = (SCRWIDTH * SCRHEIGHT) / avg;
	printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
	// handle user input
	camera.HandleInput(deltaTime);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	// animation toggle
	ImGui::Checkbox("Animate scene", &animating);
	// ray query on mouse
	Ray r = camera.GetPrimaryRay((float)mousePos.x, (float)mousePos.y);
	scene.FindNearest(r);
	bool isInScreen = mousePos.x >= 0 && mousePos.x < SCRWIDTH && mousePos.y >= 0 && mousePos.y < SCRHEIGHT;
	float4 color = isInScreen ? accumulator[mousePos.x + mousePos.y * SCRWIDTH] : float4(Color::MAGENTA);
	ImGui::ColorButton(" ", ImVec4(color.x, color.y, color.z, color.z));
	ImGui::SameLine();
	ImGui::Text("%i,%i   %i", mousePos.x, mousePos.y, r.objIdx);

	ImGui::SliderInt("ndal", &nda, 0, 3);

	if(ImGui::Button("+ PointLight"))
	{
		scene.CreatePointLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ SpotLight"))
	{
		scene.CreateSpotLight();
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

}
