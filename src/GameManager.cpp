#include "precomp.h"

#include "GameManager.h"

void GameManager::Init(Scene* scene, Renderer* renderer)
{
	m_scene = scene;
	m_renderer = renderer;

	m_state = State::START_MENU;
	m_levelIdx = 0;

	LoadLevel(0);
}

void GameManager::Tick(const float deltaTime)
{
	m_deltaTime = deltaTime;
}

void GameManager::OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord)
{
	m_windowCoordF = windowCoordF, m_windowCoord = windowCoord, m_screenCoordF = screenCoordF, m_screenCoord = screenCoord;
	if(m_isMouseLeftBtnDown || m_isMouseRightBtnDown)
	{
		m_mouseDelta = m_windowCoordF - m_mouseDownWindowPos;
		const float speed = 0.001f * m_deltaTime;
		const float3 axisSpeed = float3(m_mouseDelta.x, m_mouseDelta.y, 0) * speed;
		if(m_isMouseLeftBtnDown)
		{
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {1,0,0}, -axisSpeed.y);
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {0,1,0}, -axisSpeed.x);
		}
		else if(m_isMouseRightBtnDown)
		{
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {0,0,1}, -axisSpeed.x);
		}
		Scene::SetBlasTransform(m_scene->m_blasList[m_levelObjectInstIdx], m_scene->m_tranformList[m_levelObjectInstIdx]);
		m_scene->BuildTlas();
		m_renderer->resetAccumulator = true;
		m_mouseDownWindowPos = windowCoord;
	}
	else
	{
		m_mouseDelta = 0;
	}
	//printf("%i  %i\n", m_mouseDelta.x, m_mouseDelta.y);
}

void GameManager::OnMouseDown(const int button)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT)
	{
		m_mouseDownWindowPos = m_windowCoord;
		m_isMouseLeftBtnDown = true;
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		m_mouseDownWindowPos = m_windowCoord;
		m_isMouseRightBtnDown = true;
	}
}

void GameManager::OnMouseUp(const int button)
{
	if(button == GLFW_MOUSE_BUTTON_LEFT)
	{
		m_isMouseLeftBtnDown = false;
	}
	else if(button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		m_isMouseRightBtnDown = false;
	}
}

void GameManager::LoadLevel(const int levelIdx)
{
	Model& plane = m_scene->CreateModel(ModelType::PLANE);
	plane.m_modelData.m_meshMaterialList.front().m_type = Material::Type::DIFFUSE;
	m_scene->m_tranformList.back().m_pos = float3(0, 0, 20);
	m_scene->m_tranformList.back().m_rotAngles = float3(-90, 0, 0);
	m_scene->m_tranformList.back().m_rot = quat::fromEuler(DEG_TO_RAD(float3(-90, 0, 0)));
	m_scene->m_tranformList.back().m_scl = float3(30);
	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());

	Model& level0 = m_scene->CreateModel(ModelType::LVL_TEAPOT1, false);
	m_levelObjectInstIdx = m_scene->m_tranformList.size() - 1;
	m_scene->m_tranformList.back().m_scl = float3(0.2f);
	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());

	DirLight& dirLight = m_scene->CreateDirLight();
	dirLight.m_dir = float3(0, 0, 1);

	m_scene->BuildTlas();
}
