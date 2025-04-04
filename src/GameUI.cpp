#include "precomp.h"

#ifdef _GAME
void Renderer::GameUI()
{
	constexpr float startX = 50.0f;
	constexpr float startY = WINDOWHEIGHT * 0.5f + 100.0f;
	constexpr float btnWidth = 200.0f;
	constexpr float btnHeight = 25.0f;

	switch(m_gameManager.m_state)
	{
		case GameManager::State::START_MENU:
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(WINDOWWIDTH, WINDOWHEIGHT));
			ImGui::Begin("StartMenu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			ImGui::SetCursorPos(ImVec2(startX, startY));
			ImGui::Text("Raymatic");

			ImGui::SetCursorPosX(startX);
			if(ImGui::Button("Start", ImVec2(btnWidth, btnHeight)))
			{
				m_gameManager.ResetSceneLists();
				m_gameManager.LoadLevel(m_gameManager.m_levelIdx);
				m_gameManager.m_state = GameManager::State::GAMEPLAY;
				resetAccumulator = true;
			}

			ImGui::SetCursorPosX(startX);
			if(ImGui::Button("Quit", ImVec2(btnWidth, btnHeight)))
			{
				running = false;
			}


			ImGui::End();
			break;
		}

		case GameManager::State::WIN:
		{
			ImGui::SetNextWindowPos(ImVec2(0, 0));
			ImGui::SetNextWindowSize(ImVec2(WINDOWWIDTH, WINDOWHEIGHT));
			ImGui::Begin("WinMenu", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

			ImGui::SetCursorPos(ImVec2(startX, startY));
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
	}


}
#endif
