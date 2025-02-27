#include "precomp.h"

#include "Scene.h"

#define TINYBVH_IMPLEMENTATION
#include "tiny_bvh.h"

Scene::Scene()
{
	LoadSkydome();
	LoadModels();
}

void Scene::LoadSkydome()
{
	m_skyPixels = stbi_loadf((ASSETDIR + "Skydome/tears_of_steel_bridge_4k.hdr").c_str(), &m_skyWidth, &m_skyHeight, &m_skyBpp, 0);
	for(int i = 0; i < m_skyWidth * m_skyHeight * 3; i++) m_skyPixels[i] = sqrtf(m_skyPixels[i]);
}

void Scene::LoadModels()
{

	//Model& model = m_modelList.emplace_back(ASSETDIR + "Models/Primitives/Sphere/Sphere.obj");
	//Model& model = m_modelList.emplace_back(ASSETDIR + "Models/Primitives/SphereSmooth/SphereSmooth.glb");
	/*{
		Model& model = m_modelList.emplace_back(ASSETDIR + "Models/dragon.glb");
		printf(model.GetStrippedFileName().c_str());
		printf("\n");
		printf("NumVertices: %i\n", model.m_vertices.size());
		printf("NumTriangles: %i\n", model.m_numTriangles);
		printf("NumNormals: %i\n", model.m_normals.size());
		tinybvh::BVH& bvh = m_bvhList.emplace_back();
		bvh.Build(model.m_vertices.data(), model.m_numTriangles);
	}*/

	{
		Model& model = m_modelList.emplace_back(ASSETDIR + "Models/dragon.glb");
		printf(model.GetStrippedFileName().c_str());
		printf("\n");
		printf("NumVertices: %i\n", model.m_vertices.size());
		printf("NumTriangles: %i\n", model.m_numTriangles);
		printf("NumNormals: %i\n", model.m_normals.size());
		tinybvh::BVH& bvh = m_bvhList.emplace_back();
		bvh.Build(model.m_vertices.data(), model.m_numTriangles);
	}
}

float3 Scene::SampleSky(const Ray& ray)
{
	// sample sky
	float phi = atan2(ray.D.z, ray.D.x);
	uint u = (uint)(m_skyWidth * (phi > 0 ? phi : (phi + 2 * PI)) * INV2PI - 0.5f);
	uint v = (uint)(m_skyHeight * acos(ray.D.y) * INVPI - 0.5f);
	uint skyIdx = (u + v * m_skyWidth) % (m_skyWidth * m_skyHeight);

	m_skydomeBrightnessFactor = dbgSDBF;
	return m_skydomeBrightnessFactor * float3(m_skyPixels[skyIdx * 3], m_skyPixels[skyIdx * 3 + 1], m_skyPixels[skyIdx * 3 + 2]);
}

//TODO change to a list of all materials
Material& Scene::GetMaterial()
{
	return m_dragonMat;
}

void Scene::Intersect(Ray& ray) const
{
	m_bvhList[0].Intersect(ray); //TODO
}

float3 Scene::GetNormal(Ray& ray) const
{
	float3 n0 = m_modelList[0].m_normals[ray.hit.prim * 3];
	float3 n1 = m_modelList[0].m_normals[ray.hit.prim * 3 + 1];
	float3 n2 = m_modelList[0].m_normals[ray.hit.prim * 3 + 2];
	float w = 1.0f - ray.hit.u - ray.hit.v;
	return float3((w * n0) + (ray.hit.u * n1) + (ray.hit.v * n2));
}

PointLight& Scene::CreatePointLight()
{
	m_pointLightList.emplace_back();
	PointLight& light = m_pointLightList.back();
	return light;
}

SpotLight& Scene::CreateSpotLight()
{
	m_spotLightList.emplace_back();
	SpotLight& light = m_spotLightList.back();
	return light;
}

DirLight& Scene::CreateDirLight()
{
	m_dirLightList.emplace_back();
	DirLight& light = m_dirLightList.back();
	return light;
}

QuadLight& Scene::CreateQuadLight()
{
	m_quadLightList.emplace_back(m_nextIdx++);
	QuadLight& light = m_quadLightList.back();
	return light;
}
