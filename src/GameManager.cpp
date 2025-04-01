#include "precomp.h"

#include "GameManager.h"

void GameManager::Init(Scene* scene)
{
	m_scene = scene;

	m_state = State::START_MENU;
	m_levelIdx = 0;

	m_winQuatList[0] = quat::FromAxisAngle({0,1,0}, 90);

	LoadLevel(0);
}

void GameManager::Tick(float deltaTime)
{
}

void GameManager::LoadLevel(int levelIdx)
{
	Model& plane = m_scene->CreateModel(ModelType::PLANE);
	plane.m_modelData.m_meshMaterialList.front().m_type = Material::Type::DIFFUSE;
	m_scene->m_tranformList.back().m_pos = float3(0, 0, 20);
	m_scene->m_tranformList.back().m_rotAngles = float3(-90, 0, 0);
	m_scene->m_tranformList.back().m_rot = quat::fromEuler(DEG_TO_RAD(float3(-90, 0, 0)));
	m_scene->m_tranformList.back().m_scl = float3(30);
	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());


	Model& level0 = m_scene->CreateModel(ModelType::LVL_TTORUS, true);
	m_scene->m_tranformList.back().m_scl = float3(0.2f);
	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());

	DirLight& dirLight = m_scene->CreateDirLight();
	dirLight.m_dir = float3(0, 0, 1);

	m_scene->BuildTlas();
}
