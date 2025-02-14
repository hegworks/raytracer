#include "precomp.h"

#include "scene.h"

namespace Tmpl8
{

Scene::Scene()
{
	// we store all primitives in one continuous buffer
#ifdef FOURLIGHTS
	for(int i = 0; i < 4; i++) quad[i] = Quad(0, 0.5f);	// 0: four light sources
#else
	quad = Quad(0, 1);									// 0: light source
#endif
	sphere = Sphere(1, float3(0), 0.6f);				// 1: bouncing ball
	sphere.m_material.m_albedo = Color::GREEN;
	sphere2 = Sphere(2, float3(0, 2.5f, -3.07f), 8);	// 2: rounded corners
	cube = Cube(3, float3(2, 0, 2), float3(1.2f));		// 3: cube
	cube.m_material.m_albedo = Color::MAGENTA;			// 3: cube
	plane[0] = Plane(4, float3(1, 0, 0), 3);			// 4: left wall
	plane[1] = Plane(5, float3(-1, 0, 0), 2.99f);		// 5: right wall
	plane[2] = Plane(6, float3(0, 1, 0), 1);			// 6: floor
	plane[3] = Plane(7, float3(0, -1, 0), 2);			// 7: ceiling
	plane[4] = Plane(8, float3(0, 0, 1), 3);			// 8: front wall
	plane[5] = Plane(9, float3(0, 0, -1), 3.99f);		// 9: back wall
	torus = Torus(10, 0.8f, 0.25f);						// 10: torus
	m_nextIdx = 11;
	SetTime(0);
	// Note: once we have triangle support we should get rid of the class
	// hierarchy: virtuals reduce performance somewhat.
	m_pointLights.reserve(5);
	m_spotLights.reserve(5);
	m_dirLights.reserve(5);
}

PointLight& Scene::CreatePointLight()
{
	m_pointLights.emplace_back();
	PointLight& light = m_pointLights.back();
	return light;
}

PointLight& Scene::CreatePointLight(float3& pos, float3& color, float& intensity)
{
	m_pointLights.emplace_back();
	PointLight& light = m_pointLights.back();
	light.m_pos = pos;
	light.m_color = color;
	light.m_intensity = intensity;
	return light;
}

SpotLight& Scene::CreateSpotLight()
{
	m_spotLights.emplace_back();
	SpotLight& light = m_spotLights.back();
	return light;
}

DirLight& Scene::CreateDirLight()
{
	m_dirLights.emplace_back();
	DirLight& light = m_dirLights.back();
	return light;
}

QuadLight& Scene::CreateQuadLight()
{
	m_quadLights.emplace_back(m_nextIdx++);
	QuadLight& light = m_quadLights.back();
	return light;
}

}
