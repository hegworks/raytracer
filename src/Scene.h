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
//#define STOCH
//#define DOD
//#define SIMD

#define PROFILE_FUNCTION() ScopedTimer timer(__FUNCTION__) // ENABLE
//#define PROFILE_FUNCTION() // DISABLE 

constexpr int vertexCount = 259 * 6 * 2 * 49 * 3;

using ModelId = uint;
using BvhId = uint;

class Scene
{
public:
	Scene();

#ifndef DOD
	std::vector<PointLight> m_pointLightList;
#else
#define TOTAL_POINT_LIGHTS 512

	// Number of PointLights
	int npl = 0;

	// position
	union { float plx[TOTAL_POINT_LIGHTS]; __m128 plx4[TOTAL_POINT_LIGHTS / 4]; };
	union { float ply[TOTAL_POINT_LIGHTS]; __m128 ply4[TOTAL_POINT_LIGHTS / 4]; };
	union { float plz[TOTAL_POINT_LIGHTS]; __m128 plz4[TOTAL_POINT_LIGHTS / 4]; };

	// albedo
	union { float plr[TOTAL_POINT_LIGHTS]; __m128 plr4[TOTAL_POINT_LIGHTS / 4]; };
	union { float plg[TOTAL_POINT_LIGHTS]; __m128 plg4[TOTAL_POINT_LIGHTS / 4]; };
	union { float plb[TOTAL_POINT_LIGHTS]; __m128 plb4[TOTAL_POINT_LIGHTS / 4]; };

	// intensity
	union { float pli[TOTAL_POINT_LIGHTS]; __m128 pli4[TOTAL_POINT_LIGHTS / 4]; };
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
