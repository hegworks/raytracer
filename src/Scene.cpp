#include "precomp.h"

#include "ModelData.h"
#include "Scene.h"

#define TINYBVH_IMPLEMENTATION
#include "tiny_bvh.h"

Scene::Scene()
{
	printf("using tiny_bvh version %i.%i.%i\n", TINY_BVH_VERSION_MAJOR, TINY_BVH_VERSION_MINOR, TINY_BVH_VERSION_SUB);

	m_modelList.reserve(NUM_MODEL_TYPES);
	m_bvhList.reserve(NUM_MODEL_TYPES);

	LoadSkydome();

#pragma region MultiObject TestScene

	{
		Model& model = CreateModel(ModelType::PLANE);
		m_tranformList.back().m_scl = float3(30, 1, 30);
		m_tranformList.back().m_pos = float3(0, -2, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		Model& model = CreateModel(ModelType::SPHERE);
		m_tranformList.back().m_pos = float3(0, 1, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		Model& model = CreateModel(ModelType::CUBE);
		model.m_modelData.m_meshMaterialList.front().m_type = Material::Type::EMISSIVE;
		m_tranformList.back().m_pos = float3(-3, 0, 0);
		m_tranformList.back().m_scl = float3(2);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		Model& model = CreateModel(ModelType::DRAGON);
		m_tranformList.back().m_pos = float3(3, 0.5, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		Model& model = CreateModel(ModelType::CORNELL);
		m_tranformList.back().m_pos = float3(0, 0, 3);
		m_tranformList.back().m_rot = float3(0, 180, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	BuildTlas();
	/*SpotLight& spotLight = CreateSpotLight();
	spotLight.m_intensity = 64.0f;
	spotLight.m_pos.y = 10;*/

#pragma endregion

#pragma region DIFFUSE_PT Lighting TestScene
	/*
	useSD = false;
	Model& plane = CreateModel(ModelType::PLANE);
	m_tranformList.back().m_scl = float3(30, 1, 30);
	m_tranformList.back().m_pos = float3(0, -2, 0);
	SetBlasTransform(m_blasList.back(), m_tranformList.back());
	BuildTlas();
	SpotLight& spotLight = CreateSpotLight();
	spotLight.m_intensity = 3.0f;
	*/
#pragma endregion

#pragma region SkydomeIllumination TestScene
	/*
	CreateModel(ModelType::PLANE);
	m_tranformList.back().m_scl = float3(40, 1, 40);
	m_tranformList.back().m_pos = float3(5, -2, 5);
	SetBlasTransform(m_blasList.back(), m_tranformList.back());
	for(int z = 0; z < 10; ++z)
	{
		for(int x = 0; x < 5; ++x)
		{
			Model& model = CreateModel(ModelType::DRAGON);
			m_tranformList.back().m_pos.x = x * 3 - 2.5;
			m_tranformList.back().m_pos.z = z * 2 - 5;
			m_tranformList.back().m_pos.y = 2;
			SetBlasTransform(m_blasList.back(), m_tranformList.back());
		}
	}
	BuildTlas();
	*/
#pragma endregion

#pragma region DepthOfField TestScene
	/*
	for(int z = 0; z < 20; ++z)
	{
		for(int x = 0; x < 5; ++x)
		{
			Model& model = CreateModel(ModelType::DRAGON);
			model.m_modelData.m_meshMaterialList[0].m_type = Material::Type::DIFFUSE;
			model.m_modelData.m_meshMaterialList[1].m_type = Material::Type::DIFFUSE;
			m_tranformList.back().m_pos.x = x * 3 - 2.5;
			m_tranformList.back().m_pos.z = z * 2 - 10;
			m_tranformList.back().m_pos.y = 2;
			SetBlasTransform(m_blasList.back(), m_tranformList.back());
		}
	}
	BuildTlas();
	CreateDirLight();
	*/
#pragma endregion

#pragma region StochasticLight TestScene
	/*
	Model& plane = CreateModel(ModelType::PLANE);
	plane.m_modelData.m_meshMaterialList[0].m_type = Material::Type::DIFFUSE;
	m_tranformList.back().m_scl = float3(40, 1, 60);
	m_tranformList.back().m_pos = float3(0, -1, 20);
	SetBlasTransform(m_blasList.back(), m_tranformList.back());
	BuildTlas();

	float3 colors[3] = {float3(1,0,0),float3(0,1,0),float3(0,0,1)};
	{
		int numRows = 5;
		int numColumns = 5;
		for(int z = 0; z < numColumns; ++z)
		{
			for(int x = 0; x < numRows; ++x)
			{
				PointLight& light = CreatePointLight();
				light.m_pos = float3(x * 2 - numRows, 0, z * 2 - numRows / 2);
				light.m_color = colors[(x + z) % 3];
				light.m_intensity = 3.0;
			}
		}
	}
	{
		int numRows = 5;
		int numColumns = 5;
		for(int z = 0; z < numColumns; ++z)
		{
			for(int x = 0; x < numRows; ++x)
			{
				SpotLight& light = CreateSpotLight();
				light.m_pos = float3((x * 2) - numRows + 1, 0, (z * 2) - (numRows / 2) + 1);
				light.m_color = colors[(x + z) % 3];
				light.m_intensity = 3.0;
			}
		}
	}
	{
		int numRows = 5;
		int numColumns = 5;
		for(int z = 0; z < numColumns; ++z)
		{
			for(int x = 0; x < numRows; ++x)
			{
				QuadLight& light = CreateQuadLight();
				light.m_quad.m_pos = float3((x * -2) - numRows + 2, 0, (z * -2) - (numRows / 2) + 2);
				light.m_quad.T = mat4::Translate(light.m_quad.m_pos);
				light.m_quad.invT = light.m_quad.T.FastInvertedTransformNoScale();
				light.m_color = colors[(x + z) % 3];
				light.m_intensity = 3.0;
			}
		}
	}
	*/
#pragma endregion
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

void Scene::SetBlasTransform(tinybvh::BLASInstance& blas, Transform& t)
{
	mat4 mat =
		mat4::Translate(t.m_pos) *
		mat4::RotateX(DEG_TO_RAD(t.m_rot.x)) *
		mat4::RotateY(DEG_TO_RAD(t.m_rot.y)) *
		mat4::RotateZ(DEG_TO_RAD(t.m_rot.z)) *
		mat4::Scale(t.m_scl);
	for(int i = 0; i < 16; ++i)
	{
		blas.transform[i] = mat.cell[i];
	}
	t.m_invT = mat.Inverted().Transposed();
}

void Scene::BuildTlas()
{
	m_tlas.Build(m_blasList.data(), static_cast<int>(m_blasList.size()), m_bvhBaseList.data(), static_cast<int>(m_bvhBaseList.size()));
}

// this function is based on https://jacco.ompf2.com/2022/06/03/how-to-build-a-bvh-part-9a-to-the-gpu/
float3 Scene::SampleSky(const Ray& ray)
{
	float phi = atan2(ray.D.z, ray.D.x);
	uint u = static_cast<uint>(m_skyWidth * (phi > 0 ? phi : (phi + 2 * PI)) * INV2PI - 0.5f);
	uint v = static_cast<uint>(m_skyHeight * acos(ray.D.y) * INVPI - 0.5f);
	uint skyIdx = (u + v * m_skyWidth) % (m_skyWidth * m_skyHeight);

	m_skydomeBrightnessFactor = dbgSDBF;
	return m_skydomeBrightnessFactor * float3(m_skyPixels[skyIdx * 3], m_skyPixels[skyIdx * 3 + 1], m_skyPixels[skyIdx * 3 + 2]);
}

Material& Scene::GetMaterial(const Ray& ray)
{
	Model& model = m_modelList[m_blasList[ray.hit.inst].blasIdx];
	const int matIdx = model.VertexToMeshIdx(ray.hit.prim * 3);
	return model.m_modelData.m_meshMaterialList[matIdx];
}

Model& Scene::CreateModel(ModelType modelType)
{
	for(int i = 0; i < m_modelList.size(); ++i)
	{
		if(m_modelList[i].m_modelData.m_type == modelType)
		{
			m_blasList.emplace_back(i);
			m_tranformList.emplace_back();
			BuildTlas();
			return m_modelList[i];
		}
	}
	Model& model = m_modelList.emplace_back(ModelData::GetAddress(modelType));
	model.m_modelData.m_type = modelType;
	int verticesListSize = static_cast<int>(model.m_modelData.m_vertices.size());
	tinybvh::BVH8_CPU& bvh = m_bvhList.emplace_back();
	bvh.BuildHQ(model.m_modelData.m_vertices.data(), verticesListSize / 3);
	m_bvhBaseList.push_back(&bvh);
	int moddelListSize = static_cast<int>(m_modelList.size());
	m_blasList.emplace_back(moddelListSize - 1);
	m_tranformList.emplace_back();
	BuildTlas();

	printf("NumVertices: %llu\n", model.m_modelData.m_vertices.size());
	printf("NumMeshes: %llu\n", model.m_modelData.m_meshVertexBorderList.size());

	return model;
}

void Scene::Intersect(Ray& ray) const
{
	if(m_blasList.empty()) return;
	m_tlas.Intersect(ray);
}

bool Scene::IsOccluded(const Ray& ray)
{
	if(m_blasList.empty()) return false;
	return m_tlas.IsOccluded(ray);
}

float3 Scene::GetSmoothNormal(Ray& ray) const
{
	float3 n0 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertexDataList[ray.hit.prim * 3].m_normal;
	float3 n1 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertexDataList[ray.hit.prim * 3 + 1].m_normal;
	float3 n2 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertexDataList[ray.hit.prim * 3 + 2].m_normal;
	float w = 1.0f - ray.hit.u - ray.hit.v;
	float3 n = float3((w * n0) + (ray.hit.u * n1) + (ray.hit.v * n2));
	n = tinybvh::tinybvh_transform_vector(n, m_tranformList[m_blasList[ray.hit.inst].blasIdx].m_invT.cell);
	return normalize(n);
}

float3 Scene::GetRawNormal(Ray& ray) const
{
	float3 p0 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertices[ray.hit.prim * 3 + 0];
	float3 p1 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertices[ray.hit.prim * 3 + 1];
	float3 p2 = m_modelList[m_blasList[ray.hit.inst].blasIdx].m_modelData.m_vertices[ray.hit.prim * 3 + 2];
	float3 v0 = p1 - p0;
	float3 v1 = p2 - p0;
	float3 n = cross(v0, v1);
	n = tinybvh::tinybvh_transform_vector(n, m_tranformList[m_blasList[ray.hit.inst].blasIdx].m_invT.cell);
	return normalize(n);
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
