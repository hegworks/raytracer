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
	std::vector<PointLight> m_pointLightList;
	std::vector<SpotLight> m_spotLightList;
	std::vector<DirLight> m_dirLightList;
	std::vector<QuadLight> m_quadLightList;

	std::vector<Model> m_modelList;
	std::vector<tinybvh::BVH> m_bvhList;

	int m_nextIdx = 0;

	void LoadModels();
	void Intersect(Ray& ray);

	PointLight& CreatePointLight();
	SpotLight& CreateSpotLight();
	DirLight& CreateDirLight();
	QuadLight& CreateQuadLight();
};
