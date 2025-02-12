#include "precomp.h"

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
	float3 I = ray.O + ray.t * ray.D; // SameAs ray.IntersectionPoint()
	float3 N = scene.GetNormal(ray.objIdx, I, ray.D);
	float3 albedo = scene.GetAlbedo(ray.objIdx, I);

	float3 pointLightPos = scene.GetPointLightPos();
	float3 vIL = pointLightPos - I; // vector from Intersection to PointLight
	float tLI = length(scene.GetPointLightPos() - I); // distance between Light & Intersection
	float att = 1 / tLI * tLI; // attenuation
	float cosa = max(0.0f, dot(N, normalize(vIL)));
	float3 pointLightValue = scene.GetPointLightColor();

	const float EPSILON = 1e-4f;

	float shadowRayLength = tLI - EPSILON * 2;
	float3 shadowRayPos = I + N * EPSILON;
	Ray shadowRay(shadowRayPos, normalize(vIL), shadowRayLength);
	if(scene.IsOccluded(shadowRay))
	{
		pointLightValue = float3(0);
	}

	if(nda == 0)
	{
		return (N + 1) * 0.5f;
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
		return albedo * pointLightValue * att * cosa;
	}

	// check isOcculuded "ray" and ligthPos
	// if true: return 0
	// else:
	// lightColor = N.L stuff here
	// return albedo * lightColor
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
	//printf("%5.2fms (%.1ffps) - %.1fMrays/s\n", avg, fps, rps / 1000);
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
	ImGui::Text("Object id: %i", r.objIdx);

	ImGui::SliderInt("ndal", &nda, 0, 3);

	if(ImGui::CollapsingHeader("PointLight"))
	{
		ImGui::DragFloat3("Pos", &scene.pointLight.pos.x, 0.01f);
		ImGui::DragFloat3("Color", &scene.pointLight.color.x, 0.01f, 0.0f, 10.0f);
	}
}
