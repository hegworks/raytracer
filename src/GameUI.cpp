#include "precomp.h"

#ifdef _GAME
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
			constexpr float startY = centerY;

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
				if(showTutorial)
					m_gameManager.m_state = GameManager::State::TUTORIAL;
				else
					m_gameManager.m_state = GameManager::State::GAMEPLAY;
				resetAccumulator = true;
			}

			ImGui::SetCursorPosX(startX);
			ImGui::Checkbox("Tutorial", &showTutorial);

			ImGui::SetCursorPos(ImVec2(startX, ImGui::GetCursorPosY() + 50));
			if(ImGui::Button("Quit", ImVec2(btnWidth, btnHeight)))
			{
				running = false;
			}

			ImGui::SetCursorPos(ImVec2(10, WINDOWHEIGHT - textSize.y - 10));
			ImGui::Text("Created by Hesam Ghadimi @BUAS");


			ImGui::End();
			break;
		}

		case GameManager::State::WIN:
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(WINDOWWIDTH, WINDOWHEIGHT));
			ImGui::Begin("WinMenu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			ImGui::SetCursorPos(ImVec2(100, centerY - btnHeight * 0.5f));
			if(ImGui::Button("Next Level", ImVec2(btnWidth, btnHeight)))
			{
				m_gameManager.ResetSceneLists();
				m_gameManager.ResetGameplayStates();
				m_gameManager.m_levelIdx++;
				m_gameManager.LoadLevel(m_gameManager.m_levelIdx);
				m_gameManager.m_state = GameManager::State::GAMEPLAY;
				resetAccumulator = true;
			}


			ImGui::End();
			break;
		}


		case GameManager::State::GAMEPLAY:
		{
			constexpr float progressBarWidth = 600.0f;
			constexpr float progressBarHeight = 100.0f;
			ImGui::SetNextWindowPos(ImVec2(WINDOWWIDTH * 0.5f - progressBarWidth * 0.5f, 20));
			ImGui::SetNextWindowSize(ImVec2(progressBarWidth, progressBarHeight));
			ImGui::SetNextWindowBgAlpha(0.55f);
			ImGui::Begin("ProgressBar", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.2f));  // Background color
			ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 1, 1, 1));  // Fill color
			ImGui::ProgressBar(progress, ImVec2(-1.0f, 30.0f), "");
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
					const string text = "Welcome to RAYMATIC!";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "A CPU-based Ray-Traced game about Shadow-Art.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Hold and drag the left mouse button to contorl the pitch & yaw of the model,";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "And hold and drag the right mouse button to control the roll.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Try to create meaningful shadows from meaningless shapes!";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "Get help from the bar on top to see how close you are.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				ImGui::SetCursorPosX(centerX - (btnWidth * 0.5f));
				if(ImGui::Button("Ok!", ImVec2(btnWidth, btnHeight)))
				{
					m_gameManager.m_state = GameManager::State::GAMEPLAY;
					m_gameManager.m_tutorialStage++;
				}
			}
			else if(m_gameManager.m_tutorialStage == 1)
			{
				ImGui::SetCursorPosY(centerY - 150);
				{
					const string text = "Congratulations on completing the first level!";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Before proceeding to the next level, you can examine the original model";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				{
					const string text = "by rotating it around, using the same mouse controls.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				{
					const string text = "Note that some levels require the shadow to be in the correct roll, while some others don't.";
					const ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
					ImGui::SetCursorPosX(centerX - (textSize.x * 0.5f));
					ImGui::Text(text.c_str());
				}
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 50.0f);
				ImGui::SetCursorPosX(centerX - (btnWidth * 0.5f));
				if(ImGui::Button("Ok!", ImVec2(btnWidth, btnHeight)))
				{
					m_gameManager.m_state = GameManager::State::WIN;
					m_gameManager.m_tutorialStage++;
				}
			}


			ImGui::End();
			break;
		}
	}



}
#endif
