#include "precomp.h"

#include "GameManager.h"

void GameManager::Init(Scene* scene, Renderer* renderer)
{
	m_scene = scene;
	m_renderer = renderer;

	m_renderer->useACM = true;
	useAA = true;

	m_state = State::START_MENU;
	m_levelIdx = 0;

	const uint time = static_cast<uint>(std::chrono::system_clock::now().time_since_epoch().count());
	m_seed = m_rng.InitSeed(time);
	InitSeed(time * 17);

	PerlinGenerator::m_numOctaves = 1;
	PerlinGenerator::m_amplitude = 0.99f;
	//PerlinGenerator::m_persistence = 0.9f;

	LoadLevel(0);
}

void GameManager::Tick(const float deltaTime)
{
	m_deltaTime = deltaTime;
	if(m_isGameWon && !m_isWinSlerpFinished)
	{
		quat& q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
		q = quat::slerp2(q, quat::identity(), WIN_SLERP_SPEED * deltaTime);
		if(CalcProgress() > WIN_SLERP_END_PROGRESS)
		{
			q = quat::identity();
			m_isWinSlerpFinished = true;
		}
		OnLevelRotationUpdated();
		UpdateProgressBar(CalcProgress());
	}
}

float GameManager::CalcProgress() const
{
	switch(m_winType)
	{
		case WinType::ANY_ROT:
			return ease_in_circular(CalcProgressByAnyRot());
		case WinType::SINGLE_SIDED:
			return range_to_range(0.2f, 1.0f, 0.0f, 1.0f, CalcProgressByFixedRot(m_singleSidedWinData.m_winRotDeg, m_singleSidedWinData.m_winRotWeights));
		case WinType::DOUBLE_SIDED:
			const float p0 = CalcProgressByFixedRot(m_doubleSidedWinData.m_winRotDeg0, m_doubleSidedWinData.m_winRotWeights0);
			const float p1 = CalcProgressByFixedRot(m_doubleSidedWinData.m_winRotDeg1, m_doubleSidedWinData.m_winRotWeights1);
			return range_to_range(0.505f, 1.0f, -0.05f, 1.0f, max(p0, p1));
	}
	throw std::runtime_error("Unhandled WinType");
}

float GameManager::CalcProgressByFixedRot(const float3& targetRotDeg, const float3& weight) const
{
	const quat& q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
	const float3 qdeg = RAD_TO_DEG(q.toEuler());
	const float3 wdeg = targetRotDeg;
	float3 diff = abs(abs(wdeg) - abs(qdeg));
	diff = DEG_TO_RAD(diff);
	return 1.0f - ((weight.x * diff.x + weight.y * diff.y + weight.z * diff.z) / PI);
}

float GameManager::CalcProgressByAnyRot() const
{
	quat q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
	q.normalize();
	const float3 qn = q * float3(0.0f, 0.0f, -1.0f);

	quat w = m_anyRotWinData.m_winQuat;
	w.normalize();
	const float3 wn = w * float3(0.0f, 0.0f, -1.0f);

	return abs(clamp(dot(qn, wn), -1.0f, 1.0f));
}

void GameManager::OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord)
{
	if(m_isGameWon) return;

	m_windowCoordF = windowCoordF, m_windowCoord = windowCoord, m_screenCoordF = screenCoordF, m_screenCoord = screenCoord;
	if(m_isMouseLeftBtnDown || m_isMouseRightBtnDown)
	{
		m_mouseDelta = m_windowCoordF - m_mouseDownWindowPos;
		const float3 axisSpeed = float3(m_mouseDelta.x, m_mouseDelta.y, 0) * DRAG_ROTATE_SPEED * m_deltaTime;
		if(m_isMouseLeftBtnDown)
		{
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {1,0,0}, -axisSpeed.y);
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {0,1,0}, -axisSpeed.x);
		}
		else if(m_isMouseRightBtnDown)
		{
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[m_levelObjectInstIdx], {0,0,1}, -axisSpeed.x);
		}
		OnLevelRotationUpdated();
		m_mouseDownWindowPos = windowCoord;

		const float progress = CalcProgress();
		UpdateProgressBar(progress);
		//printf("%f\n", progress);
		if(progress > WIN_PERCENTAGE)
		{
			m_isGameWon = true;
		}
	}
	else
	{
		m_mouseDelta = 0;
	}
}

void GameManager::OnLevelRotationUpdated() const
{
	Scene::SetBlasTransform(m_scene->m_blasList[m_levelObjectInstIdx], m_scene->m_tranformList[m_levelObjectInstIdx]);
	m_scene->BuildTlas();
	m_renderer->resetAccumulator = true;
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

void GameManager::OnKeyDown(const int key) const
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
#if 1
	while(true)
	{
		RotateRandomly();
		if(CalcProgress() < leastDiff)
			return;
	}
#else
	float min = 1.0f;
	while(true)
	{
		RotateRandomly();
		float prog = CalcProgress();
		if(prog < min)
			min = prog, printf("%f\n", min);
	}
#endif
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

#if 0 // square level
	Model& level0 = m_scene->CreateModel(ModelType::LVL_SQUARE, false);
	m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
	m_scene->m_tranformList.back().m_scl = float3(0.2f);

	m_winType = WinType::ANY_ROT;
	m_anyRotWinData.m_winQuat = quat::identity();
#elif 0 // ttorus level
	Model& level0 = m_scene->CreateModel(ModelType::LVL_TTORUS, false);
	m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
	m_scene->m_tranformList.back().m_scl = float3(0.2f);

	m_winType = WinType::ANY_ROT;
	m_anyRotWinData.m_winQuat = quat::identity();
#elif 0 // unused situation
	m_winType = WinType::SINGLE_SIDED;
	m_singleSidedWinData.m_winRotDeg = 0;
	m_singleSidedWinData.m_winRotWeights = 0.33f;
#elif 0 // teapot level 1
	Model& level0 = m_scene->CreateModel(ModelType::LVL_TEAPOT1, true);
	m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
	m_scene->m_tranformList.back().m_scl = float3(0.2f);

	m_winType = WinType::DOUBLE_SIDED;
	m_doubleSidedWinData.m_winRotDeg0 = 0;
	m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
	m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
	m_doubleSidedWinData.m_winRotWeights1 = 0.33f;
#elif 1 // teapot level 0
	Model& level0 = m_scene->CreateModel(ModelType::LVL_TEAPOT0, true);
	m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
	m_scene->m_tranformList.back().m_scl = float3(1.2f);

	m_winType = WinType::DOUBLE_SIDED;
	m_doubleSidedWinData.m_winRotDeg0 = 0;
	m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
	m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
	m_doubleSidedWinData.m_winRotWeights1 = 0.33f;
#endif

	RotateUntilLeastDiff(0.01f);
	UpdateProgressBar(CalcProgress());

	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());

	DirLight& dirLight = m_scene->CreateDirLight();
	dirLight.m_dir = float3(0, 0, 1);

	m_scene->BuildTlas();
}
