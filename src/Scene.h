#pragma once

#include "DirLight.h"
#include "Model.h"
#include "PointLight.h"
#include "QuadLight.h"
#include "SpotLight.h"

class Scene
{
public:
	std::vector<Model> m_models;
	std::vector<tinybvh::BVH> m_bvhs;
	std::vector<PointLight> m_pointLights;
	std::vector<SpotLight> m_spotLights;
	std::vector<DirLight> m_dirLights;
	std::vector<QuadLight> m_quadLights;

	int m_nextIdx = 0;

	void LoadModels();

	PointLight& CreatePointLight();
	SpotLight& CreateSpotLight();
	DirLight& CreateDirLight();
	QuadLight& CreateQuadLight();
};
