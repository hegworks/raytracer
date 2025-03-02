#include "precomp.h"

#include "Scene.h"

#define TINYBVH_IMPLEMENTATION
#include "ModelData.h"
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

void Scene::SetBlasTransform(tinybvh::BLASInstance& blas, const mat4& mat)
{
	for(int i = 0; i < 15; ++i)
	{
		blas.transform[i] = mat.cell[i];
	}
}

void Scene::SetBlasTransform(tinybvh::BLASInstance& blas, const Transform& t)
{
	mat4 mat =
		mat4::Translate(t.m_pos) *
		mat4::RotateX(DEG_TO_RAD(t.m_rot.x)) *
		mat4::RotateY(DEG_TO_RAD(t.m_rot.y)) *
		mat4::RotateZ(DEG_TO_RAD(t.m_rot.z)) *
		mat4::Scale(t.m_scl);
	for(int i = 0; i < 15; ++i)
	{
		blas.transform[i] = mat.cell[i];
	}
}

void Scene::BuildTlas()
{
	m_tlas.Build(m_blasList.data(), m_blasList.size(), m_bvhBaseList.data(), m_bvhBaseList.size());
}

void Scene::LoadModels()
{
	m_modelList.reserve(NUM_MODEL_TYPES);
	m_bvhList.reserve(NUM_MODEL_TYPES);
	CreateModel(ModelType::DRAGON);
	CreateModel(ModelType::SPHERE);
	int y = 0;
	int z = 0;
	int x = 0;
	/*for(int i = 1; i < 21; ++i)
	{
		CreateModel(ModelType::SPHERE);
		{
			mat4 t = mat4::Translate(x, y, z);
			x += 3;
			SetBlasTransform(m_blasList.back(), t);
		}
		if(i % 4 == 0)
		{
			x = 0;
			z += 2;
		}
	}
	x = 0;
	y = 2;
	z = 0;
	for(int i = 1; i < 21; ++i)
	{
		CreateModel(ModelType::DRAGON);
		{
			mat4 t = mat4::Translate(x, y, z);
			y += 2;
			SetBlasTransform(m_blasList.back(), t);
		}
		if(i % 4 == 0)
		{
			y = 2;
			x += 2;
		}
	}*/
	BuildTlas();
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

Material& Scene::GetMaterial(Ray& ray)
{
	Model& model = m_modelList[m_blasList[ray.hit.inst].blasIdx];
	int matIdx = model.VertexToMeshIdx(ray.hit.prim * 3);
	return model.m_modelData.m_meshMaterialList[matIdx];
}

Model& Scene::CreateModel(ModelType modelType)
{
	for(int i = 0; i < m_modelList.size(); ++i)
	{
		if(m_modelList[i].m_modelData.m_type == modelType)
		{
			m_blasList.emplace_back(i);
			BuildTlas();
			return m_modelList[i];
		}
	}
	Model& model = m_modelList.emplace_back(ModelData::GetAddress(modelType));
	model.m_modelData.m_type = modelType;
	tinybvh::BVH& bvh = m_bvhList.emplace_back(model.m_modelData.m_vertices.data(), model.m_modelData.m_vertices.size() / 3);
	m_bvhBaseList.push_back(&bvh);
	m_blasList.emplace_back(m_modelList.size() - 1);
	m_tranformList.emplace_back();
	BuildTlas();

	printf("NumVertices: %i\n", model.m_modelData.m_vertices.size());
	printf("NumMeshes: %i\n", model.m_modelData.m_meshVertexBorderList.size());

	return model;
}

void Scene::Intersect(Ray& ray) const
{
	m_tlas.Intersect(ray);
}

bool Scene::IsOccluded(const Ray& ray)
{
	return m_tlas.IsOccluded(ray);
}

float3 Scene::GetNormal(Ray& ray) const
{
	float3 n0 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertexDataList[ray.hit.prim * 3].m_normal;
	float3 n1 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertexDataList[ray.hit.prim * 3 + 1].m_normal;
	float3 n2 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertexDataList[ray.hit.prim * 3 + 2].m_normal;
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
