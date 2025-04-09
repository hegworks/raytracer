#include "precomp.h"

#ifdef _GAME
static string GetLevelModelTypeName(const ModelType modelType)
{
	switch(modelType)
	{
		case ModelType::LVL_SQUARE: return "Cube";
		case ModelType::LVL_BUCKET: return "Bucket";
		case ModelType::LVL_COCKTAIL: return "Cocktail";
		case ModelType::LVL_BALLOON_DOG: return "Balloon Dog";
		case ModelType::LVL_CHAIR: return "Chair";
		case ModelType::LVL_SPINNER: return "Spinner";
		case ModelType::LVL_CAT: return "Cat";
		case ModelType::LVL_DRAGON: return "Dragon";
		case ModelType::LVL_GUITAR: return "Guitar";
		case ModelType::LVL_RAYMATIC: return "RAYMATIC!";
		default: printf("Unhandled modelType: %hhu", static_cast<uint8_t>(modelType)), throw runtime_error("Unhandled modelType");
	}
	throw runtime_error("Unhandled modelType");
}

void Renderer::GameUI()
{
	constexpr float centerX = WINDOWWIDTH * 0.5f;
	constexpr float centerY = WINDOWHEIGHT * 0.5f;
	constexpr float btnWidth = 200.0f;
	constexpr float btnHeight = 40.0f;

	switch(m_gameManager.m_state)
	{
		case GameManager::State::START_MENU:
		{
			constexpr float startX = 50.0f;
			constexpr float startY = centerY - 50.0f;

			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(WINDOWWIDTH, WINDOWHEIGHT));
			ImGui::Begin("StartMenu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			const ImVec2 textSize = ImGui::CalcTextSize("RAYMATIC");
			ImGui::SetCursorPos(ImVec2(startX + (btnWidth * 0.5f) - (textSize.x * 0.5f), startY));
			ImGui::Text("RAYMATIC");

			ImGui::SetCursorPosX(startX);
			if(ImGui::Button("Start", ImVec2(btnWidth, btnHeight)))
			{
				m_gameManager.ResetSceneLists();
				m_gameManager.LoadLevel(m_gameManager.m_levelIdx);
				if(m_gameManager.m_showTutorial)
					m_gameManager.m_state = GameManager::State::TUTORIAL;
				else
					m_gameManager.m_state = GameManager::State::GAMEPLAY;
				resetAccumulator = true;
			}

			ImGui::SetCursorPosX(startX);
			ImGui::Checkbox("Tutorial", &m_gameManager.m_showTutorial);

			ImGui::SetCursorPosX(startX);
			ImGui::Checkbox("Guide bar", &m_gameManager.m_showProgressBar);

			ImGui::SetCursorPos(ImVec2(startX, ImGui::GetCursorPosY() + 50));
			if(ImGui::Button("Quit", ImVec2(btnWidth, btnHeight)))
			{
				running = false;
			}

			ImGui::SetCursorPos(ImVec2(10, WINDOWHEIGHT - textSize.y - 10));
			ImGui::Text("Created by Hesam Ghadimi @ BUAS");


			ImGui::End();
			break;
		}

		case GameManager::State::WIN:
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(WINDOWWIDTH - 350, WINDOWHEIGHT));
			ImGui::Begin("WinMenu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			// Draw background
			ImGui::GetWindowDrawList()->AddRectFilled(
				ImVec2(centerX - 150.0f, 25.0f),
				ImVec2(centerX + 150.0f, 60.0f),
				IM_COL32(100, 100, 100, 100) // RGBA
			);
			{
				const string text = to_string(m_gameManager.m_levelIdx + 1) + "/" + to_string(GameManager::NUM_LEVELS) + " - " + GetLevelModelTypeName(m_gameManager.m_levelObjectModelType);
				const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
				ImGui::SetCursorPos(ImVec2(centerX - (textSize.x * 0.5f), 25.0f));
				ImGui::Text(text.c_str());
			}

			ImGui::SetCursorPos(ImVec2(25.0f, 25.0f));
			if(ImGui::Button("Next Level", ImVec2(btnWidth, btnHeight)))
			{
				m_gameManager.m_levelIdx++;
				m_gameManager.ResetSceneLists();
				m_gameManager.ResetGameplayStates();
				if(m_gameManager.m_levelIdx != GameManager::NUM_LEVELS)
				{
					m_gameManager.LoadLevel(m_gameManager.m_levelIdx);
					m_gameManager.m_state = GameManager::State::GAMEPLAY;
				}
				else
				{
					m_gameManager.m_tutorialStage = 2;
					m_gameManager.m_levelIdx = 0;
					m_gameManager.m_state = GameManager::State::TUTORIAL;
				}
				resetAccumulator = true;
			}
			ImGui::End();
			break;
		}

		case GameManager::State::GAMEPLAY:
		{
			if(!m_gameManager.m_showProgressBar) { ImGui::End(); break; }

			constexpr float progressBarWidth = 600.0f;
			constexpr float progressBarHeight = 100.0f;
			ImGui::SetNextWindowPos(ImVec2(WINDOWWIDTH * 0.5f - progressBarWidth * 0.5f, 17));
			ImGui::SetNextWindowSize(ImVec2(progressBarWidth, progressBarHeight));
			ImGui::SetNextWindowBgAlpha(0.55f);
			ImGui::Begin("ProgressBar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.2f));  // Background color
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 1, 1, 1));  // Fill color
			ImGui::ProgressBar(m_gameManager.m_progress, ImVec2(-1.0f, 30.0f), "");
			ImGui::PopStyleColor(2);


			ImGui::End();
			break;
		}

		case GameManager::State::TUTORIAL:
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(WINDOWWIDTH, WINDOWHEIGHT));
			ImGui::SetNextWindowBgAlpha(0.65f);
			ImGui::Begin("StartMenu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);


			if(m_gameManager.m_tutorialStage == 0)
			{
				ImGui::SetCursorPosY(centerY - 175);
				{
					const string text = "Welcome to RAYMATIC - where form finds meaning in shadow.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "A shadow-sculpting puzzle powered by CPU ray tracing.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Left-click & drag: spin the shape (pitch & yaw)";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "Right-click & drag: twist it (roll)";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Your goal: cast meaningful shadows from meaningless shapes";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "The bar at the top shows how close you are to the target.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				ImGui::SetCursorPosX(centerX - (btnWidth * 0.5f));
				if(ImGui::Button("Begin", ImVec2(btnWidth, btnHeight)))
				{
					m_gameManager.m_state = GameManager::State::GAMEPLAY;
					m_gameManager.m_tutorialStage++;
				}
			}
			else if(m_gameManager.m_tutorialStage == 1)
			{
				ImGui::SetCursorPosY(centerY - 150);
				{
					const string text = "Nice work! First shadow unlocked.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Before diving into the next puzzle, take a moment to explore the original form.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "Use the same mouse controls to rotate and inspect it.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Note that some levels require the correct roll. Others don't.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				ImGui::SetCursorPosX(centerX - (btnWidth * 0.5f));
				if(ImGui::Button("Understood", ImVec2(btnWidth, btnHeight)))
				{
					m_gameManager.m_state = GameManager::State::WIN;
					m_gameManager.m_tutorialStage++;
				}
			}
			else if(m_gameManager.m_tutorialStage == 2)
			{
				ImGui::SetCursorPosY(centerY - 150);
				{
					const string text = "End of the demo - thanks for playing.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Now try again with the guide bar off, and trust in the shadow.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "The full version will let you load and play with your own custom models.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "Stay tuned.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				ImGui::SetCursorPosX(centerX - (btnWidth * 0.5f));
				if(ImGui::Button("Main Menu", ImVec2(btnWidth, btnHeight)))
				{
					m_gameManager.m_tutorialStage = 0;
					m_gameManager.LoadStartMenu();
					m_gameManager.m_state = GameManager::State::START_MENU;
					resetAccumulator = true;
				}
			}


			ImGui::End();
			break;
		}
	}

}
#endif
