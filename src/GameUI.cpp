#include "precomp.h"

#ifdef _GAME
void Renderer::GameUI()
{
	const float topWidth = 600.0f;
	const float topHeight = 100.0f;
	ImGui::SetNextWindowPos(ImVec2(WINDOWWIDTH * 0.5f - topWidth * 0.5f, 20));
	ImGui::SetNextWindowSize(ImVec2(topWidth, topHeight));
	ImGui::SetNextWindowBgAlpha(0.55f);
	ImGui::Begin("Bottom", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

	ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0, 0, 0, 0.2f));  // Background color
	ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1, 1, 1, 1));  // Fill color
	ImGui::ProgressBar(progress, ImVec2(-1.0f, 30.0f), "");
	ImGui::PopStyleColor(2);

	ImGui::End();
}
#endif
