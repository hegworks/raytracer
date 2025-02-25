#include "precomp.h"

#include "Scene.h"

#define TINYBVH_IMPLEMENTATION
#include "tiny_bvh.h"

void Scene::LoadModels()
{
	Model& model = m_models.emplace_back(ASSETDIR + "Models/Primitives/Cube/Cube.obj");
	//Model& model = m_models.emplace_back(ASSETDIR + "Models/Primitives/Sphere/Sphere.obj");
	//Model& model = m_models.emplace_back(ASSETDIR + "Models/Primitives/SphereSmooth/SphereSmooth.glb");
	printf(model.GetStrippedFileName().c_str());
	printf("\n");
	printf("NumMeshes: %i\n", model.m_meshes.size());
	printf("NumVertices: %i\n", model.m_meshes[0].m_vertices.size());
	printf("NumTriangles: %i\n", model.m_meshes[0].m_triangles.size());
	printf("NumNormals: %i\n", model.m_meshes[0].m_normals.size());

	{
		tinybvh::BVH& bvh = m_bvhs.emplace_back();
		bvh.Build(model.m_meshes[0].m_triangles.data(), model.m_meshes[0].m_triangles.size() / 3);
	}

}

PointLight& Scene::CreatePointLight()
{
	m_pointLights.emplace_back();
	PointLight& light = m_pointLights.back();
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
