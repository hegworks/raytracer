#include "precomp.h"

#include "Scene.h"

#define TINYBVH_IMPLEMENTATION
#include "tiny_bvh.h"

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

void Scene::Intersect(Ray& ray)
{
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
