#pragma once

#include "DirLight.h"
#include "Model.h"
#include "PointLight.h"
#include "QuadLight.h"
#include "SpotLight.h"
#include "Transform.h"

//#define SPHERE_FLAKE
#define NUMLIGHTS 128
#define STOCH_SAMPLES 128
#define SUPPORTED_POINT_LIGHTS 512

//#define STOCH

//#define SCALAR
//#define DOD
#define SIMD
//#define AVX

//#define PROFILE_FUNCTION() ScopedTimer timer(__FUNCTION__) // ENABLE
#define PROFILE_FUNCTION() // DISABLE 

constexpr int vertexCount = 259 * 6 * 2 * 49 * 3;

using ModelId = uint;
using BvhId = uint;

class Scene
{
public:
	Scene();
	int npl = 0; // Number of PointLights

#ifdef SCALAR
	std::vector<PointLight> m_pointLightList;

#elif defined(DOD)
	// position
	float plx[SUPPORTED_POINT_LIGHTS];
	float ply[SUPPORTED_POINT_LIGHTS];
	float plz[SUPPORTED_POINT_LIGHTS];
	// albedo
	float plr[SUPPORTED_POINT_LIGHTS];
	float plg[SUPPORTED_POINT_LIGHTS];
	float plb[SUPPORTED_POINT_LIGHTS];
	// intensity
	float pli[SUPPORTED_POINT_LIGHTS];

#elif defined(SIMD) // SSE
	// position
	union { float plx[SUPPORTED_POINT_LIGHTS]; __m128 plx4[SUPPORTED_POINT_LIGHTS / 4]; };
	union { float ply[SUPPORTED_POINT_LIGHTS]; __m128 ply4[SUPPORTED_POINT_LIGHTS / 4]; };
	union { float plz[SUPPORTED_POINT_LIGHTS]; __m128 plz4[SUPPORTED_POINT_LIGHTS / 4]; };
	// albedo
	union { float plr[SUPPORTED_POINT_LIGHTS]; __m128 plr4[SUPPORTED_POINT_LIGHTS / 4]; };
	union { float plg[SUPPORTED_POINT_LIGHTS]; __m128 plg4[SUPPORTED_POINT_LIGHTS / 4]; };
	union { float plb[SUPPORTED_POINT_LIGHTS]; __m128 plb4[SUPPORTED_POINT_LIGHTS / 4]; };
	// intensity
	union { float pli[SUPPORTED_POINT_LIGHTS]; __m128 pli4[SUPPORTED_POINT_LIGHTS / 4]; };

#elif defined(AVX)
	// position
	union { float plx[SUPPORTED_POINT_LIGHTS]; __m256 plx8[SUPPORTED_POINT_LIGHTS / 8]; };
	union { float ply[SUPPORTED_POINT_LIGHTS]; __m256 ply8[SUPPORTED_POINT_LIGHTS / 8]; };
	union { float plz[SUPPORTED_POINT_LIGHTS]; __m256 plz8[SUPPORTED_POINT_LIGHTS / 8]; };
	// albedo
	union { float plr[SUPPORTED_POINT_LIGHTS]; __m256 plr8[SUPPORTED_POINT_LIGHTS / 8]; };
	union { float plg[SUPPORTED_POINT_LIGHTS]; __m256 plg8[SUPPORTED_POINT_LIGHTS / 8]; };
	union { float plb[SUPPORTED_POINT_LIGHTS]; __m256 plb8[SUPPORTED_POINT_LIGHTS / 8]; };
	// intensity
	union { float pli[SUPPORTED_POINT_LIGHTS]; __m256 pli8[SUPPORTED_POINT_LIGHTS / 8]; };

#endif

	std::vector<SpotLight> m_spotLightList;
	std::vector<DirLight> m_dirLightList;
	std::vector<QuadLight> m_quadLightList;

	void Intersect(Ray& ray) const;
	bool IsOccluded(const Ray& ray);
	float3 GetSmoothNormal(Ray& ray) const;
	float3 GetRawNormal(Ray& ray) const;
	float3 CalculateSmoothNormal(Ray& ray) const;
	float3 SampleSky(const Ray& ray);
	Model& GetModel(const Ray& ray);
	Material& GetMaterial(const Ray& ray);
	float3 GetAlbedo(const Ray& ray, Model& model);

	Model& CreateModel(ModelType modelType);

	void CreatePointLight();
	SpotLight& CreateSpotLight();
	DirLight& CreateDirLight();
	QuadLight& CreateQuadLight();
	void CreateSphreFlake(float x, float y, float z, float s, int d = 0);

	void SetBlasTransform(tinybvh::BLASInstance& blas, Transform& t);
	void BuildTlas();

private:
	void LoadSkydome();
	void SetBlasTransform(tinybvh::BLASInstance& blas, const mat4& mat);

	int m_nextIdx = 0;

	float* m_skyPixels = nullptr;
	int m_skyWidth = 0;
	int m_skyHeight = 0;
	float m_skyWidthF = 0;
	float m_skyHeightF = 0;
	int m_skyBpp = 0;
	uint m_skySize = 0;
	float m_skydomeBrightnessFactor = 0.8f;

#ifdef SPHERE_FLAKE
	float4 vertices[259 * 6 * 2 * 49 * 3];
	float3 normals[259 * 6 * 2 * 49 * 3];
	int verts = 0;
	int norms = 0;
#endif

private:
	std::vector<tinybvh::BVH8_CPU> m_bvhList;
	std::vector<tinybvh::BVHBase*> m_bvhBaseList;
	tinybvh::BVH m_tlas;
public:
	std::vector<Model> m_modelList;
	std::vector<tinybvh::BLASInstance> m_blasList;
	std::vector<Transform> m_tranformList;
};
