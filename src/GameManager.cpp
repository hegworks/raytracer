#include "precomp.h"

#include "GameManager.h"

void GameManager::Init(Scene* scene, Renderer* renderer)
{
	m_scene = scene;
	m_renderer = renderer;

	m_state = State::START_MENU;
	m_levelIdx = 0;

	m_seed = m_rng.InitSeed(std::chrono::system_clock::now().time_since_epoch().count());

	LoadLevel(0);
}

void GameManager::Tick(const float deltaTime)
{
	m_deltaTime = deltaTime;
}

float GiveDiff(const quat& q, const float3& targetRotDeg, const float3& weight)
{
	const float3 qa = RAD_TO_DEG(q.toEuler());
	const float3 diff = DEG_TO_RAD(float3(abs(targetRotDeg - qa)));
	return  1.0f - ((weight.x * diff.x + weight.y * diff.y + weight.z * diff.z) / PI);
}

float GiveDiff2(const quat& q, const float3& targetRotDeg, const float3& weight)
{
	const float3 qdeg = RAD_TO_DEG(q.toEuler());
	const float3 wdeg = targetRotDeg;
	float3 diff = abs(abs(wdeg) - abs(qdeg));
	diff = DEG_TO_RAD(diff);
	return 1.0f - ((weight.x * diff.x + weight.y * diff.y + weight.z * diff.z) / PI);
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

		quat& q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
#if 0 // all rots
		float3 r = q * float3(0, 0, -1);
		m_renderer->progress = abs(dot(r, m_winRotDeg));
#elif 0 // exact 
		m_renderer->progress = abs(dot(m_winQuat, q));
#elif 0 
		float p0 = abs(dot(m_winQuat, q));
		float p1 = abs(dot(m_winQuat2, q));
		m_renderer->progress = max(p0, p1);
#elif 0
		const float angleDiff = RAD_TO_DEG(acos(abs(dot(m_winQuat, q))));
		m_renderer->progress = range_to_range(90, 0, 0, 1, angleDiff);
#elif 0
		float3 eu = RAD_TO_DEG(q.toEuler());
		float3 winEu = m_winRotDeg;
		float3 diff = DEG_TO_RAD(float3(abs(winEu - eu)));
		m_renderer->progress = 1.0f - ((m_winWeights.x * diff.x + m_winWeights.y * diff.y + m_winWeights.z * diff.z) / PI);
#elif 0
		float diff0 = GiveDiff(q, m_winRotDeg, m_winWeights);
		float diff1 = GiveDiff(q, m_winRotDeg2, m_winWeights);
		m_renderer->progress = max(diff0, diff1);
#elif 0 // all rots
		const float3 qn = q * float3(0, 0, -1);
		const float3 wn = m_winQuat * float3(0, 0, -1);
		m_renderer->progress = abs(dot(qn, wn));
#elif 1
		const float diff0 = GiveDiff2(q, m_winRotDeg, m_winWeights);
		const float diff1 = GiveDiff2(q, m_winRotDeg2, m_winWeights);
		m_renderer->progress = max(diff0, diff1);
#endif
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

void GameManager::OnKeyDown(const int key)
{
	if(key == GLFW_KEY_SPACE)
	{
		printf("q:\n");
		const quat& q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
		cout << "(" << q.w << "," << q.x << "," << q.y << "," << q.z << ")\n";

		printf("n:\n");
		const float3 n = q * float3(0, 0, -1);
		cout << "(" << n.x << "," << n.y << "," << n.z << ")\n";

		printf("eu:\n");
		const float3 eu = RAD_TO_DEG(q.toEuler());
		cout << "(" << eu.x << "," << eu.y << "," << eu.z << ")\n";

		printf("\n");
	}
}

float GameManager::CalculateWinProgress() const
{
	const quat& q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
	const float diff0 = GiveDiff2(q, m_winRotDeg, m_winWeights);
	const float diff1 = GiveDiff2(q, m_winRotDeg2, m_winWeights);
	return max(diff0, diff1);
}

void GameManager::RotateRandomly()
{
	const float3 randRot =
	{
		DEG_TO_RAD(m_rng.RandomFloat(m_seed,0,180)),
		DEG_TO_RAD(m_rng.RandomFloat(m_seed,0,180)),
		DEG_TO_RAD(m_rng.RandomFloat(m_seed,0,180))
	};
	Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {1,0,0}, randRot.x);
	Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {0,1,0}, randRot.y);
	Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {0,0,1}, randRot.z);
}

void GameManager::RotateUntilLeastDiff(const float leastDiff)
{
	while(true)
	{
		RotateRandomly();
		const float similarity = CalculateWinProgress();
		printf("%f\n", similarity);
		if(similarity < leastDiff)
		{
			return;
		}
	}
}

void GameManager::UpdateProgressBar(const float progress) const
{
	m_renderer->progress = progress;
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

	Model& level0 = m_scene->CreateModel(ModelType::LVL_TEAPOT1, true);
	m_levelObjectInstIdx = m_scene->m_tranformList.size() - 1;
	m_scene->m_tranformList.back().m_scl = float3(0.2f);
	m_winQuat = quat::identity();
	m_winQuat2 = quat(0, 0, 1, 0);
	m_winRotDeg = float3(0, 0, 0);
	m_winRotDeg2 = float3(180, 0, 180);
	m_winWeights = float3(0.33f, 0.33f, 0.33f);
	RotateUntilLeastDiff(0.55f);
	UpdateProgressBar(CalculateWinProgress());

	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());

	DirLight& dirLight = m_scene->CreateDirLight();
	dirLight.m_dir = float3(0, 0, 1);

	m_scene->BuildTlas();
}
