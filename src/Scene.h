#pragma once

#include <unordered_map>

#include "DirLight.h"
#include "Model.h"
#include "PointLight.h"
#include "QuadLight.h"
#include "SpotLight.h"
#include "Transform.h"

//#define SPHERE_FLAKE
//#define PLS // PointLightSIMD

constexpr int vertexCount = 259 * 6 * 2 * 49 * 3;

using ModelId = uint;
using BvhId = uint;

class Scene
{
public:
	Scene();

#ifndef PLS
	std::vector<PointLight> m_pointLightList;
#else
	std::vector<float> plx, ply, plz;
	std::vector<float> plr, plg, plb;
	std::vector<float> pli;
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
