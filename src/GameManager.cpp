#include "precomp.h"

#ifdef _GAME

#include "GameManager.h"

#include "CountdownTimer.h"

void GameManager::Init(Scene* scene, Renderer* renderer)
{
	m_scene = scene;
	m_renderer = renderer;

	m_renderer->useACM = true;
	useAA = true;
	useDOF = false;
	defocusAngle = 1.0f;
	focusDistance = 10.2f;

	m_state = State::START_MENU;
	m_levelIdx = 0;

	const uint time = static_cast<uint>(std::chrono::system_clock::now().time_since_epoch().count());
	m_seed = m_rng.InitSeed(time);
	InitSeed(time * 17);

	PerlinGenerator::m_numOctaves = 4;
	PerlinGenerator::m_amplitude = 0.9f;
	PerlinGenerator::m_persistence = 0.9f;

	m_scaleTimer = new CountdownTimer(SCALE_TIME, false);

	m_renderer->font = ImGui::GetIO().Fonts->AddFontFromFileTTF((ASSETDIR + "Fonts/Omniblack.ttf").c_str(), 32.0f);

	LoadStartMenu();
	//LoadLevel(0);
}

void GameManager::Tick(const float deltaTime)
{
	m_deltaTime = deltaTime;
	if(m_isGameWon && !m_isWinSlerpFinished)
	{
		quat& q = m_scene->m_tranformList[m_levelObjectInstIdx].m_rot;
		q = quat::slerp(q, m_winQuat, WIN_SLERP_SPEED * deltaTime);
		float progress = lerp(m_winTimeProgress, 1.0f, WIN_SLERP_SPEED * deltaTime);
		m_winTimeProgress = progress;
		if(CalcProgressByQuat(q, m_winQuat) > WIN_SLERP_END_PROGRESS)
		{
			q = m_winQuat;
			m_isWinSlerpFinished = true;
			m_scaleTimer->Reset();
			progress = 1.0f;
		}
		OnTransformChanged(m_levelObjectInstIdx);
		UpdateProgressBar(progress);
	}
	else if(m_isWinSlerpFinished && !m_isShrinkDeformedFinished)
	{
		float3& scl = m_scene->m_tranformList[m_levelObjectInstIdx].m_scl;
		m_scaleTimer->Update(deltaTime);
		scl = lerp(m_levelObjectScale, EPS, ease_in_out_elastic(m_scaleTimer->GetProgress()));
		if(scl.x < EPS)
		{
			scl = EPS;
			m_isShrinkDeformedFinished = true;
			m_scaleTimer->Reset();
		}
		OnTransformChanged(m_levelObjectInstIdx);

	}
	else if(m_isShrinkDeformedFinished && !m_isGrowFullFinished)
	{
		const float targetScale = m_levelObjectScale * SCALE_FACTOR;
		float3& scl = m_scene->m_tranformList[m_levelObjectInstIdx + 1].m_scl;
		m_scaleTimer->Update(deltaTime);
		scl = lerp(EPS, targetScale, ease_out_bounce(m_scaleTimer->GetProgress()));
		if(scl.x > targetScale)
		{
			scl = targetScale;
			m_isGrowFullFinished = true;
			useDOF = true;
			if(m_levelIdx == 0 && m_showTutorial)
				m_state = State::TUTORIAL;
			else
			{
				m_state = State::WIN;
				if(m_levelObjectModelType == ModelType::LVL_SPINNER) dbgSDBF = 0.1f, m_scene->m_dirLightList.clear();
				else dbgSDBF = dbgSDBF_DEFAULT;
			}
		}
		OnTransformChanged(m_levelObjectInstIdx + 1);
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

float GameManager::CalcProgressByQuat(quat a, quat b)
{
	a.normalize();
	const float3 an = a * float3(0.0f, 0.0f, -1.0f);

	b.normalize();
	const float3 bn = b * float3(0.0f, 0.0f, -1.0f);

	return clamp(dot(an, bn), -1.0f, 1.0f);
}

void GameManager::OnMouseMove(const float2& windowCoordF, const int2& windowCoord, const float2& screenCoordF, const int2& screenCoord)
{
	if(m_state != State::GAMEPLAY && m_state != State::WIN) return;
	if(m_isGameWon && !m_isGrowFullFinished) return;

	m_windowCoordF = windowCoordF, m_windowCoord = windowCoord, m_screenCoordF = screenCoordF, m_screenCoord = screenCoord;
	if(m_isMouseLeftBtnDown || m_isMouseRightBtnDown)
	{
		const int objIdx = m_levelObjectInstIdx + (m_isGameWon ? 1 : 0);
		m_mouseDelta = m_windowCoordF - m_mouseDownWindowPos;
		const float3 axisSpeed = float3(m_mouseDelta.x, m_mouseDelta.y, 0) * INV_SCRSCALE * DRAG_ROTATE_SPEED * m_deltaTime;
		if(m_isMouseLeftBtnDown)
		{
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[objIdx], {1,0,0}, -axisSpeed.y);
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[objIdx], {0,1,0}, -axisSpeed.x);
		}
		else if(m_isMouseRightBtnDown)
		{
			Renderer::RotateAroundWorldAxis(m_scene->m_tranformList[objIdx], {0,0,1}, -axisSpeed.x);
		}
		OnTransformChanged(objIdx);
		m_mouseDownWindowPos = windowCoord;

		if(!m_isGameWon)
		{
			const float progress = CalcProgress();
			UpdateProgressBar(progress);
			//printf("%f\n", progress);
			if(progress > WIN_PERCENTAGE)
			{
				const float p0 = CalcProgressByFixedRot(0, 0.33f);
				const float p1 = CalcProgressByFixedRot(float3(180, 0, 180), 0.33f);
				m_winQuat = p0 >= p1 ? quat::identity() : quat(0, 0, 1, 0);

				m_isGameWon = true;
				m_winTimeProgress = progress;
			}
		}
	}
	else
	{
		m_mouseDelta = 0;
	}
}

void GameManager::OnTransformChanged(const int instanceIdx) const
{
	Scene::SetBlasTransform(m_scene->m_blasList[instanceIdx], m_scene->m_tranformList[instanceIdx]);
	m_scene->BuildTlas();
	m_renderer->resetAccumulator = true;
}

void GameManager::OnMouseDown(const int button)
{
	if(ImGui::GetIO().WantCaptureMouse && m_state != State::WIN) return; //TODO remove this before release

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

void GameManager::UpdateProgressBar(const float progress)
{
	m_progress = progress;
}

void GameManager::ResetSceneLists()
{
	m_scene->m_tranformList.clear();
	m_scene->m_blasList.clear();
	m_scene->m_modelList.clear();
	m_scene->m_dirLightList.clear();
	m_scene->m_bvhBaseList.clear();
	m_scene->m_bvhList.clear();
	m_scene->m_pointLightList.clear();
	m_scene->m_dirLightList.clear();
	m_scene->m_spotLightList.clear();
	m_scene->m_quadLightList.clear();
}

void GameManager::ResetGameplayStates()
{
	m_isGameWon = false;
	m_isWinSlerpFinished = false;
	m_isShrinkDeformedFinished = false;
	m_isGrowFullFinished = false;
	m_scaleTimer->ForceEnd();
}

void GameManager::LoadLevel(const int levelIdx)
{
	dbgSDBF = dbgSDBF_DEFAULT;
	useDOF = false;

	DirLight& frontLight = m_scene->CreateDirLight();
	frontLight.m_dir = float3(0, 0, 1);

	//DirLight& downLight = m_scene->CreateDirLight();

	//DirLight& leftLight = m_scene->CreateDirLight();
	//leftLight.m_dir = {-1,0,0};

	//DirLight& rightLight = m_scene->CreateDirLight();
	//rightLight.m_dir = {1,0,0};

	Model& scenery = m_scene->CreateModel(ModelType::SCN_ROOM_LEVEL);
	for(Material& material : scenery.m_modelData.m_meshMaterialList)
	{
		if(strcmp(material.m_name, "window_glass") == 0)
		{
			material.m_type = Material::Type::REFRACTIVE;
			material.m_albedo = {0,0,1};
			material.m_factor0 = 0.03f;
			material.m_factor1 = 1.33f;
		}
		if(strcmp(material.m_name, "shadow_bg") == 0)
		{
			material.m_type = Material::Type::DIFFUSE;
			material.m_albedo = 1.0f;
		}
	}
	m_scene->m_tranformList.back().m_pos = float3(-8.1f, -10.05f, -4.0f);
	//m_scene->m_tranformList.back().m_rotAngles = float3(-90, 0, 0);
	//m_scene->m_tranformList.back().m_rot = quat::fromEuler(DEG_TO_RAD(float3(-90, 0, 0)));
	m_scene->m_tranformList.back().m_scl = float3(1.8f);
	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());

	switch(levelIdx)
	{
		case 0:
		{
			m_levelObjectModelType = ModelType::LVL_SQUARE;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_SQUARE, false, false, true);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 0.135f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(0, 1, 0) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_SQUARE_FULL, false, true, false);
			for(Material& material : fullShape.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = {0,1,0};
				material.m_factor0 = 0.985f;
				material.m_factor1 = 0.5f;
			}
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::ANY_ROT;
			m_anyRotWinData.m_winQuat = quat::identity();


			break;
		}

		case 1:
		{
			m_levelObjectModelType = ModelType::LVL_COCKTAIL;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_COCKTAIL, true, false, true, Axis::Y, Model::RandType::FIXED_MOVE);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 1.2f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(1, 0, 1) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_COCKTAIL, false, true, false);
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				if(strcmp(material.m_name, "cocktail") == 0)
				{
					material.m_factor0 = 3.0f;
					break;
				}
			}
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::DOUBLE_SIDED;
			m_doubleSidedWinData.m_winRotDeg0 = 0;
			m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
			m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
			m_doubleSidedWinData.m_winRotWeights1 = 0.33f;


			break;
		}

		case 2:
		{
			m_levelObjectModelType = ModelType::LVL_SPINNER;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_SPINNER, true, false, true, Axis::Y, Model::RandType::FIXED_MOVE);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 1.0f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(0.5f) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_SPINNER, false, true, false);
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::ANY_ROT;
			m_anyRotWinData.m_winQuat = quat::identity();


			break;
		}

		case 3:
		{
			m_levelObjectModelType = ModelType::LVL_RAYMATIC;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_RAYMATIC, true, false, true);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 0.65f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(1.0f) * DEFAULT_ALBEDO;
				material.m_factor0 = 1.0f;
				material.m_factor1 = 1.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_RAYMATIC, false, true, false);
			for(Material& material : fullShape.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::REFRACTIVE;
				material.m_factor0 = 1.25f;
				material.m_factor1 = 1.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::SINGLE_SIDED;
			m_singleSidedWinData.m_winRotDeg = 0;


			break;
		}

		case 4:
		{
			m_levelObjectModelType = ModelType::LVL_DRAGON;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_DRAGON, true, false, true);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 1.2f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(1, 0, 0) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_DRAGON, false, true, false);
			for(Material& material : fullShape.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::REFRACTIVE;
				material.m_albedo = 1.0f - float3(1, 0, 0);
				material.m_factor0 = 2.0f;
				material.m_factor1 = 1.5f;
			}
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::DOUBLE_SIDED;
			m_doubleSidedWinData.m_winRotDeg0 = 0;
			m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
			m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
			m_doubleSidedWinData.m_winRotWeights1 = 0.33f;


			break;
		}

		case 5:
		{
			m_levelObjectModelType = ModelType::LVL_BALLOON_DOG;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_BALLOON_DOG, true, false, false);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 0.7f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(0, 0, 1) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_BALLOON_DOG, false, true, false);
			for(Material& material : fullShape.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(0, 0, 1);
				material.m_factor0 = 0.7f;
				material.m_factor1 = 0.8f;
			}
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::DOUBLE_SIDED;
			m_doubleSidedWinData.m_winRotDeg0 = 0;
			m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
			m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
			m_doubleSidedWinData.m_winRotWeights1 = 0.33f;


			break;
		}

		case 6:
		{
			m_levelObjectModelType = ModelType::LVL_BUCKET;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_BUCKET, true, false, true, Axis::Z);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 1.0f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(0.5f) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_BUCKET, false, true, false);
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::DOUBLE_SIDED;
			m_doubleSidedWinData.m_winRotDeg0 = 0;
			m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
			m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
			m_doubleSidedWinData.m_winRotWeights1 = 0.33f;


			break;
		}

		case 7:
		{
			m_levelObjectModelType = ModelType::LVL_CAT;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_CAT, true, false, true);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 1.2f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(1, 1, 1) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_CAT, false, true, false);
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::DOUBLE_SIDED;
			m_doubleSidedWinData.m_winRotDeg0 = 0;
			m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
			m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
			m_doubleSidedWinData.m_winRotWeights1 = 0.33f;


			break;
		}

		case 8:
		{
			m_levelObjectModelType = ModelType::LVL_GUITAR;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_GUITAR, true, false, true);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 0.7f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(0.5f) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_GUITAR, false, true, false);
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::ANY_ROT;
			m_anyRotWinData.m_winQuat = quat::identity();


			break;
		}

		case 9:
		{
			m_levelObjectModelType = ModelType::LVL_CHAIR;
			Model& lvlObj = m_scene->CreateModel(ModelType::LVL_CHAIR, true, false, true, Axis::Y);
			m_levelObjectInstIdx = static_cast<int>(m_scene->m_tranformList.size()) - 1;
			m_levelObjectScale = 1.0f;
			for(Material& material : lvlObj.m_modelData.m_meshMaterialList)
			{
				material.m_type = Material::Type::PATH_TRACED;
				material.m_albedo = float3(1, 1, 1) * DEFAULT_ALBEDO;
				material.m_factor0 = 0.0f;
				material.m_factor1 = 0.0f;
			}
			m_scene->m_tranformList.back().m_scl = float3(m_levelObjectScale);

			Model& fullShape = m_scene->CreateModel(ModelType::LVL_CHAIR, false, true, false);
			m_scene->m_tranformList.back().m_scl = float3(EPS);

			m_winType = WinType::DOUBLE_SIDED;
			m_doubleSidedWinData.m_winRotDeg0 = 0;
			m_doubleSidedWinData.m_winRotDeg1 = float3(180, 0, 180);
			m_doubleSidedWinData.m_winRotWeights0 = 0.33f;
			m_doubleSidedWinData.m_winRotWeights1 = 0.33f;


			break;
		}


		default:
			throw std::runtime_error("Unhandled levelIdx");
	}

	// unused situation
	//m_winType = WinType::SINGLE_SIDED;
	//m_singleSidedWinData.m_winRotDeg = 0;
	//m_singleSidedWinData.m_winRotWeights = 0.33f;

	RotateUntilLeastDiff(0.01f);
	UpdateProgressBar(CalcProgress());

	Scene::SetBlasTransform(m_scene->m_blasList[m_levelObjectInstIdx], m_scene->m_tranformList[m_levelObjectInstIdx]);
	Scene::SetBlasTransform(m_scene->m_blasList[m_levelObjectInstIdx + 1], m_scene->m_tranformList[m_levelObjectInstIdx + 1]);
	m_scene->BuildTlas();
}

void GameManager::LoadStartMenu() const
{
	Model& mainMenu = m_scene->CreateModel(ModelType::SCN_ROOM_MAIN);
	m_scene->m_tranformList.back().m_pos = float3(-3.2f, -1.2f, -3.9f);
	m_scene->m_tranformList.back().m_rotAngles = float3(20.0f, 35.0f, 10.0f);
	m_scene->m_tranformList.back().m_rot = quat::fromEuler(DEG_TO_RAD(m_scene->m_tranformList.back().m_rotAngles));
	Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());
	for(Material& material : mainMenu.m_modelData.m_meshMaterialList)
	{
		if(strcmp(material.m_name, "raymatic") == 0)
		{
			material.m_type = Material::Type::REFRACTIVE;
			material.m_factor0 = 15.0f;
			material.m_factor1 = 1.0;
		}
		else if(strcmp(material.m_name, "glass ball") == 0)
		{
			material.m_type = Material::Type::REFRACTIVE;
			material.m_albedo = 1.0f;
			material.m_factor0 = 0.0f;
			material.m_factor1 = 1.54f;
		}
		else if(strcmp(material.m_name, "window") == 0)
		{
			material.m_type = Material::Type::REFRACTIVE;
			//material.m_albedo = 1.0f;
			material.m_factor0 = 2.0f;
			material.m_factor1 = 1.54f;
		}
	}
	/*{
		m_scene->CreateModel(ModelType::DRAGON);
		m_scene->m_tranformList.back().m_pos = float3(-2, -0.8, -3);
		Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());
	}
	{
		m_scene->CreateModel(ModelType::CUBE);
		m_scene->m_tranformList.back().m_pos = float3(0, -0.8f, -3);
		Scene::SetBlasTransform(m_scene->m_blasList.back(), m_scene->m_tranformList.back());
	}
	*/
	{
		DirLight& light = m_scene->CreateDirLight();
		light.m_intensity = 0.3f;
		light.m_dir = float3(0.593f, -0.237f, 0.769f);
	}
	m_scene->BuildTlas();
}

#endif
