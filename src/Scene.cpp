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

	stbi_set_flip_vertically_on_load(false);

#pragma region SIMD TestScene
	useSD = false;
	dbgSLS = 32;

	/*{
		CreateModel(ModelType::SPHERE);
		m_tranformList.back().m_scl = float3(2.5);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}*/

	Model& plane = CreateModel(ModelType::PLANE);
	plane.m_modelData.m_meshMaterialList[0].m_type = Material::Type::DIFFUSE;
	m_tranformList.back().m_scl = float3(40, 1, 60);
	m_tranformList.back().m_pos = float3(0, 0, 20);
	SetBlasTransform(m_blasList.back(), m_tranformList.back());
	BuildTlas();

	float3 colors[3] = {float3(1,0,0),float3(0,1,0),float3(0,0,1)};
	{
		int numRows = 5;
		int numColumns = 10;
		for(int z = 0; z < numColumns; ++z)
		{
			for(int x = 0; x < numRows; ++x)
			{
#ifndef PLS
				CreatePointLight();
				PointLight& light = m_pointLightList.back();
				light.m_pos = float3(x * 2 - numRows, 1, z * 2 - numRows / 2);
				light.m_color = colors[(x + z) % 3];
				light.m_intensity = 3.0;
#else
				CreatePointLight();
				float3 pos = float3(x * 2 - numRows, 1, z * 2 - numRows / 2);
				plx.back() = pos.x;
				ply.back() = pos.y;
				plz.back() = pos.z;
				float3 color = colors[(x + z) % 3];
				plr.back() = color.x;
				plg.back() = color.y;
				plb.back() = color.z;
				pli.back() = 3.0;
#endif

			}
		}
	}
	{
		int numRows = 5;
		int numColumns = 10;
		for(int z = 0; z < numColumns; ++z)
		{
			for(int x = 0; x < numRows; ++x)
			{
#ifndef PLS
				CreatePointLight();
				PointLight& light = m_pointLightList.back();
				light.m_pos = float3(x * 2 - numRows, -1, z * 2 - numRows / 2);
				light.m_color = colors[(x + z) % 3];
				light.m_intensity = 3.0;
#else
				CreatePointLight();
				float3 pos = float3(x * 2 - numRows, -1, z * 2 - numRows / 2);
				plx.back() = pos.x;
				ply.back() = pos.y;
				plz.back() = pos.z;
				float3 color = colors[(x + z) % 3];
				plr.back() = color.x;
				plg.back() = color.y;
				plb.back() = color.z;
				pli.back() = 3.0;
#endif

			}
		}
	}

#pragma endregion

#pragma region Texture TestScene
	/*
	{
		CreateModel(ModelType::KENNY);
	}
	{
		CreateModel(ModelType::SNAKE);
		m_tranformList.back().m_scl = float3(0.165f);
		m_tranformList.back().m_pos = float3(-2, -0.8, -1);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		CreateModel(ModelType::CUBE);
		m_tranformList.back().m_pos = float3(0, -0.8f, -3);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	BuildTlas();
	*/
#pragma endregion

#ifdef SPHERE_FLAKE
	{
		CreateSphreFlake(0, 0, 0, 1);

		Model::ModelData modelData;
		modelData.m_type = ModelType::FLAKE;
		modelData.m_initialized = true;
		Material& mat = modelData.m_meshMaterialList.emplace_back();
		modelData.m_meshVertexBorderList.emplace_back(vertexCount - 1);

		for(int i = 0; i < vertexCount; ++i)
		{
			modelData.m_vertices.emplace_back(vertices[i]);
			Model::VertexData& vertexData = modelData.m_vertexDataList.emplace_back();
			vertexData.m_normal = normals[i];
		}
		Model& model = m_modelList.emplace_back(modelData);

		auto& bvh = m_bvhList.emplace_back();
		bvh.Build(vertices, vertexCount / 3);
		m_bvhBaseList.push_back(&bvh);
		int moddelListSize = static_cast<int>(m_modelList.size());
		m_blasList.emplace_back(moddelListSize - 1);
		m_tranformList.emplace_back();
		BuildTlas();

		printf("NumVertices: %i\n", vertices);
		printf("NumMeshes: %i\n", 1);
	}
#endif

#pragma region QuatRotation TestScene
	/*
	{
		Model& model = CreateModel(ModelType::DRAGON);
	}
	{
		Model& model = CreateModel(ModelType::PLANE);
		m_tranformList.back().m_scl = float3(30, 1, 30);
		m_tranformList.back().m_pos = float3(0, -2, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	BuildTlas();
	*/
#pragma endregion

#pragma region MultiObject TestScene
	/*
	{
		CreateModel(ModelType::PLANE);
		m_tranformList.back().m_scl = float3(30, 1, 30);
		m_tranformList.back().m_pos = float3(0, -2, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		CreateModel(ModelType::SPHERE);
		m_tranformList.back().m_pos = float3(0, 1, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		Model& model = CreateModel(ModelType::CUBE);
		model.m_modelData.m_meshMaterialList.front().m_type = Material::Type::EMISSIVE;
		model.m_modelData.m_meshMaterialList.front().m_albedo = 1;
		m_tranformList.back().m_pos = float3(-3, 0, 0);
		m_tranformList.back().m_scl = float3(2);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	{
		CreateModel(ModelType::DRAGON);
		m_tranformList.back().m_pos = float3(3, 0.5, 0);
		SetBlasTransform(m_blasList.back(), m_tranformList.back());
	}
	//{
	//	Model& model = CreateModel(ModelType::CORNELL);
	//	m_tranformList.back().m_pos = float3(0, 0, 3);
	//	//m_tranformList.back().m_rot = float3(0, 180, 0);
	//	SetBlasTransform(m_blasList.back(), m_tranformList.back());
	//}
	BuildTlas();
	SpotLight& spotLight = CreateSpotLight();
	spotLight.m_intensity = 64.0f;
	spotLight.m_pos.y = 10;
	*/
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
	m_skySize = m_skyWidth * m_skyHeight;
	m_skyWidthF = static_cast<float>(m_skyWidth);
	m_skyHeightF = static_cast<float>(m_skyHeight);
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
		t.m_rot.toMatrix() *
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

float3 Scene::SampleSky(const Ray& ray)
{
	const float u = m_skyWidthF * (atan2f(ray.D.z, ray.D.x) * INV2PI) - 0.5f;
	const uint x = static_cast<uint>(u);
	const float v = m_skyHeightF * (acosf(ray.D.y) * INVPI) - 0.5f;
	const uint y = static_cast<uint>(v);

	if(useBI) // Bilinear Interpolation
	{
		const uint skyIdx00 = (x + y * m_skyWidth) % (m_skySize);
		const uint skyIdx10 = ((x + 1) + y * m_skyWidth) % (m_skySize);
		const uint skyIdx01 = (x + (y + 1) * m_skyWidth) % (m_skySize);
		const uint skyIdx11 = ((x + 1) + (y + 1) * m_skyWidth) % (m_skySize);
		const float3 rgb00 = float3(m_skyPixels[skyIdx00 * 3 + 0], m_skyPixels[skyIdx00 * 3 + 1], m_skyPixels[skyIdx00 * 3 + 2]);
		const float3 rgb10 = float3(m_skyPixels[skyIdx10 * 3 + 0], m_skyPixels[skyIdx10 * 3 + 1], m_skyPixels[skyIdx10 * 3 + 2]);
		const float3 rgb01 = float3(m_skyPixels[skyIdx01 * 3 + 0], m_skyPixels[skyIdx01 * 3 + 1], m_skyPixels[skyIdx01 * 3 + 2]);
		const float3 rgb11 = float3(m_skyPixels[skyIdx11 * 3 + 0], m_skyPixels[skyIdx11 * 3 + 1], m_skyPixels[skyIdx11 * 3 + 2]);
		const float uRatio = u * m_skyWidthF - floor(u * m_skyWidthF);
		const float vRatio = v * m_skyHeightF - floor(v * m_skyHeightF);
		const float3 rgbHor = lerp(rgb00, rgb10, uRatio);
		const float3 rgbVer = lerp(rgb01, rgb11, uRatio);
		return dbgSDBF * lerp(rgbHor, rgbVer, vRatio);
	}
	else
	{
		const uint skyIdx00 = (x + y * m_skyWidth) % (m_skySize);
		return dbgSDBF * float3(m_skyPixels[skyIdx00 * 3 + 0], m_skyPixels[skyIdx00 * 3 + 1], m_skyPixels[skyIdx00 * 3 + 2]);
	}
}

Model& Scene::GetModel(const Ray& ray)
{
	return m_modelList[m_blasList[ray.hit.inst].blasIdx];
}

Material& Scene::GetMaterial(const Ray& ray)
{
	Model& model = m_modelList[m_blasList[ray.hit.inst].blasIdx];
	const uint matIdx = model.VertexToMeshIdx(ray.hit.prim * 3);
	return model.m_modelData.m_meshMaterialList[matIdx];
}

float3 Scene::GetAlbedo(const Ray& ray, Model& model)
{
	uint tri = ray.hit.prim * 3;
	const Model::VertexData& v0 = model.m_modelData.m_vertexDataList[tri + 0]; //tri
	const Model::VertexData& v1 = model.m_modelData.m_vertexDataList[tri + 1]; //tri
	const Model::VertexData& v2 = model.m_modelData.m_vertexDataList[tri + 2]; //tri
	int mesh = model.VertexToMeshIdx(tri);
	int surfaceIndex = model.m_modelData.m_surfaceIndexList[mesh];
	const Surface& tex0 = model.m_modelData.m_surfaceList[surfaceIndex];
	float2 uv = // texcoord
		ray.hit.u * v1.m_texCoord +
		ray.hit.v * v2.m_texCoord +
		(1.0f - (ray.hit.u + ray.hit.v)) * v0.m_texCoord;
	int iu = (int)(uv.x * tex0.width) % tex0.width;
	int iv = (int)(uv.y * tex0.height) % tex0.height;
	float3 texel = tex0.pixelsF[iu + iv * tex0.width];
	return texel;
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
	auto& bvh = m_bvhList.emplace_back();
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

void Scene::CreatePointLight()
{
#ifndef PLS
	m_pointLightList.emplace_back();
#else
	plx.emplace_back();
	ply.emplace_back();
	plz.emplace_back();

	plr.emplace_back();
	plg.emplace_back();
	plb.emplace_back();

	pli.emplace_back();
#endif
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
	m_quadLightList.emplace_back();
	QuadLight& light = m_quadLightList.back();
	return light;
}

#ifdef SPHERE_FLAKE
//void Scene::CreateSphreFlake(float x, float y, float z, float s, int d)
//{
//	// procedural tesselated sphere flake object
//#define P(F,a,b,c) p[i+F*64]={(float)a ,(float)b,(float)c}
//	float3 p[384], pos(x, y, z), ofs(3.5);
//	for(int i = 0, u = 0; u < 8; u++) for(int v = 0; v < 8; v++, i++)
//		P(0, u, v, 0), P(1, u, 0, v), P(2, 0, u, v),
//		P(3, u, v, 7), P(4, u, 7, v), P(5, 7, u, v);
//	for(int i = 0; i < 384; i++) p[i] = normalize(p[i] - ofs) * s + pos;
//	for(int i = 0, side = 0; side < 6; side++, i += 8)
//		for(int u = 0; u < 7; u++, i++) for(int v = 0; v < 7; v++, i++)
//		{
//			vertices[verts++] = p[i];
//			vertices[verts++] = p[i + 8];
//			vertices[verts++] = p[i + 1];
//			vertices[verts++] = p[i + 1];
//			vertices[verts++] = p[i + 9];
//			vertices[verts++] = p[i + 8];
//
//			normals[norms++] = normalize(p[i] - pos);
//			normals[norms++] = normalize(p[i + 8] - pos);
//			normals[norms++] = normalize(p[i + 1] - pos);
//			normals[norms++] = normalize(p[i + 1] - pos);
//			normals[norms++] = normalize(p[i + 9] - pos);
//			normals[norms++] = normalize(p[i + 8] - pos);
//		}
//	if(d < 3) CreateSphreFlake(x + s * 1.55f, y, z, s * 0.5f, d + 1);
//	if(d < 3) CreateSphreFlake(x - s * 1.5f, y, z, s * 0.5f, d + 1);
//	if(d < 3) CreateSphreFlake(x, y + s * 1.5f, z, s * 0.5f, d + 1);
//	if(d < 3) CreateSphreFlake(x, y - s * 1.5f, z, s * 0.5f, d + 1);
//	if(d < 3) CreateSphreFlake(x, y, z + s * 1.5f, s * 0.5f, d + 1);
//	if(d < 3) CreateSphreFlake(x, y, z - s * 1.5f, s * 0.5f, d + 1);
//}

void Scene::CreateSphreFlake(float x, float y, float z, float s, int d)
{
	// procedural tesselated sphere flake object
#define P(F,a,b,c) p[i+F*64]={(float)a ,(float)b,(float)c}

	float3 p[384], pos(x, y, z), ofs(3.5);

	// Generate the initial cube grid points
	for(int i = 0, u = 0; u < 8; u++) for(int v = 0; v < 8; v++, i++)
		P(0, u, v, 0), P(1, u, 0, v), P(2, 0, u, v),
		P(3, u, v, 7), P(4, u, 7, v), P(5, 7, u, v);

	// Project points onto sphere and scale/position
	for(int i = 0; i < 384; i++)
		p[i] = normalize(p[i] - ofs) * s + pos;

	// Create triangles for the sphere surface
	for(int i = 0, side = 0; side < 6; side++)
	{
		int baseIdx = side * 64;
		for(int u = 0; u < 7; u++)
		{
			for(int v = 0; v < 7; v++)
			{
				int idx = baseIdx + u * 8 + v;

				// First triangle (ensure correct winding order)
				vertices[verts++] = p[idx];
				vertices[verts++] = p[idx + 1];
				vertices[verts++] = p[idx + 8];

				// Normals for first triangle
				normals[norms++] = normalize(p[idx] - pos);
				normals[norms++] = normalize(p[idx + 1] - pos);
				normals[norms++] = normalize(p[idx + 8] - pos);

				// Second triangle (ensure correct winding order)
				vertices[verts++] = p[idx + 1];
				vertices[verts++] = p[idx + 9];
				vertices[verts++] = p[idx + 8];

				// Normals for second triangle
				normals[norms++] = normalize(p[idx + 1] - pos);
				normals[norms++] = normalize(p[idx + 9] - pos);
				normals[norms++] = normalize(p[idx + 8] - pos);
			}
		}
	}

	// Recursively create smaller sphere flakes if depth allows
	if(d < 3)
	{
		CreateSphreFlake(x + s * 1.55f, y, z, s * 0.5f, d + 1);
		CreateSphreFlake(x - s * 1.5f, y, z, s * 0.5f, d + 1);
		CreateSphreFlake(x, y + s * 1.5f, z, s * 0.5f, d + 1);
		CreateSphreFlake(x, y - s * 1.5f, z, s * 0.5f, d + 1);
		CreateSphreFlake(x, y, z + s * 1.5f, s * 0.5f, d + 1);
		CreateSphreFlake(x, y, z - s * 1.5f, s * 0.5f, d + 1);
	}
}
#endif
