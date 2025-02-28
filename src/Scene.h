#pragma once

#include <unordered_map>

#include "DirLight.h"
#include "Model.h"
#include "PointLight.h"
#include "QuadLight.h"
#include "SpotLight.h"

using ModelId = uint;
using BvhId = uint;

class Scene
{
public:
	Scene();

	std::vector<PointLight> m_pointLightList;
	std::vector<SpotLight> m_spotLightList;
	std::vector<DirLight> m_dirLightList;
	std::vector<QuadLight> m_quadLightList;

	void Intersect(Ray& ray) const;
	bool IsOccluded(const Ray& ray);
	float3 GetNormal(Ray& ray) const;
	float3 SampleSky(const Ray& ray);
	Material& GetMaterial(Ray& ray);

	Model& CreateModel(ModelType modelType);

	PointLight& CreatePointLight();
	SpotLight& CreateSpotLight();
	DirLight& CreateDirLight();
	QuadLight& CreateQuadLight();

private:
	void LoadModels();
	void LoadSkydome();
	void SetBlasTransform(tinybvh::BLASInstance& blas, const mat4& mat);
	void BuildTlas();

	int m_nextIdx = 0;

	float* m_skyPixels = nullptr;
	int m_skyWidth = 0;
	int m_skyHeight = 0;
	int m_skyBpp = 0;
	float m_skydomeBrightnessFactor = 0.8f;

	std::vector<tinybvh::BVH> m_bvhList;
	std::vector<tinybvh::BVHBase*> m_bvhBaseList;
	tinybvh::BVH m_tlas;
public: //TODO temporary public for debugging
	std::vector<Model> m_modelList;
	std::vector<tinybvh::BLASInstance> m_blasList;

	Material m_dragonMat; //TODO change to a list of all materials
};
