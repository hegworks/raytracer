#include "precomp.h"

#include "renderer.h"

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	if(tdd)
	{
		ImGui::Begin("2D Debugger");

		ImGui::DragFloat("Scale", &tddSceneScale, 0.01f, -5, 5);
		ImGui::DragInt2("Offset", &tddOffset.x);
		ImGui::DragInt("Ray Count", &tddrx, 0.5f, 1, 200);

		ImGui::Checkbox("Black Background", &tddBBG);
		ImGui::Checkbox("Primary Ray", &tddPRay);
		ImGui::Checkbox("Primary Ray Length", &tddPRayL);
		ImGui::Checkbox("P Normal", &tddPN);
		ImGui::Checkbox("P Normal Length", &tddPNL);
		ImGui::Separator();
		ImGui::Checkbox("Point Light Pos", &tddPLP);
		ImGui::Checkbox("Point Light Rays", &tddPLR);

		ImGui::End();
	}

	// animation toggle
	ImGui::Text("avg	fps	rps");
	ImGui::Text("%.1f	%.0f	%.0f", davg, dfps, drps);
	ImGui::SameLine();
	if(ImGui::Button("ResetCam"))
	{
		tddResetCam = true;
	}

	ImGui::Checkbox("Animate", &animating);
	ImGui::SameLine();
	ImGui::Checkbox("AA", &useAA);
	ImGui::SameLine();
	ImGui::Checkbox("TDD", &tdd);

	// ray query on mouse
	Ray r = camera.GetPrimaryRay((float)mousePos.x, (float)mousePos.y);
	scene.FindNearest(r);
	bool isInScreen = mousePos.x >= 0 && mousePos.x < SCRWIDTH && mousePos.y >= 0 && mousePos.y < SCRHEIGHT;
	uint pixel = 0xFF00FF, red = 0xFFFFFF, green = 0xFFFFFF, blue = 0xFFFFFF;
	if(isInScreen)
	{
		pixel = screen->pixels[mousePos.x + mousePos.y * SCRWIDTH];
		red = (pixel & 0xFF0000) >> 16;
		green = (pixel & 0x00FF00) >> 8;
		blue = pixel & 0x0000FF;
	}
	ImVec4 color = isInScreen ? ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f) : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	ImGui::ColorButton("", color);
	ImGui::SameLine();

	ImGui::Text("%u,%u,%u  %i,%i  %i", red, green, blue, mousePos.x, mousePos.y, r.objIdx);

	ImGui::SliderInt("ndal", &ndal, 0, 3);

	ImGui::Checkbox("ACM MAX", &useACMMax);
	ImGui::SameLine();
	ImGui::SliderInt(" ", &acmMax, 1, 1000);

	// credits to Okke for the idea of creating lights at runtime
	if(ImGui::Button("+ PointL"))
	{
		scene.CreatePointLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ SpotL"))
	{
		scene.CreateSpotLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ DirL"))
	{
		scene.CreateDirLight();
	}
	ImGui::SameLine();
	if(ImGui::Button("+ QuadL"))
	{
		scene.CreateQuadLight();
	}

	if(!scene.m_pointLights.empty())
	{
		if(ImGui::CollapsingHeader("PointLights"))
		{
			for(int i = 0; i < static_cast<int>(scene.m_pointLights.size()); i++)
			{
				if(ImGui::TreeNode(("PL " + std::to_string(i)).c_str()))
				{
					ImGui::DragFloat3("Pos", &scene.m_pointLights[i].m_pos.x, 0.01f);
					ImGui::ColorEdit3("Color", &scene.m_pointLights[i].m_color.x);
					ImGui::DragFloat("Intensity", &scene.m_pointLights[i].m_intensity, 0.01f, 0.0f, 1000.0f);

					ImGui::TreePop();
				}
			}
		}
	}
	if(!scene.m_spotLights.empty())
	{
		if(ImGui::CollapsingHeader("SpotLights"))
		{
			for(int i = 0; i < static_cast<int>(scene.m_spotLights.size()); i++)
			{
				if(ImGui::TreeNode(("SL " + std::to_string(i)).c_str()))
				{
					ImGui::DragFloat3("Pos", &scene.m_spotLights[i].m_pos.x, 0.01f);
					ImGui::ColorEdit3("Color", &scene.m_spotLights[i].m_color.x);
					ImGui::DragFloat("Intensity", &scene.m_spotLights[i].m_intensity, 0.01f, 0.0f, 1000.0f);
					float3 dir = scene.m_spotLights[i].m_dir;
					ImGui::DragFloat3("Dir", &dir.x, 0.001f, -1.0f, 1.0f);
					scene.m_spotLights[i].m_dir = normalize(dir);

					ImGui::DragFloat("CosI", &scene.m_spotLights[i].m_cosI, 0.001f, scene.m_spotLights[i].m_cosO, 1.0f);
					ImGui::SameLine();
					ImGui::Text("%.2f", RAD_TO_DEG(acos(scene.m_spotLights[i].m_cosI)));

					ImGui::DragFloat("CosO", &scene.m_spotLights[i].m_cosO, 0.001f, 0.0f, scene.m_spotLights[i].m_cosI);
					ImGui::SameLine();
					ImGui::Text("%.2f", RAD_TO_DEG(acos(scene.m_spotLights[i].m_cosO)));

					ImGui::TreePop();
				}
			}
		}
	}
	if(!scene.m_dirLights.empty())
	{
		if(ImGui::CollapsingHeader("DirLights"))
		{
			for(int i = 0; i < static_cast<int>(scene.m_dirLights.size()); i++)
			{
				if(ImGui::TreeNode(("DL " + std::to_string(i)).c_str()))
				{
					DirLight& light = scene.m_dirLights[i];
					float3 dir = light.m_dir;
					ImGui::DragFloat3("Dir", &dir.x, 0.01f);
					light.m_dir = normalize(dir);
					ImGui::ColorEdit3("Color", &light.m_color.x);
					ImGui::DragFloat("Intensity", &light.m_intensity, 0.01f, 0.0f, 1000.0f);

					ImGui::TreePop();
				}
			}
		}
	}
	if(!scene.m_quadLights.empty())
	{
		if(ImGui::CollapsingHeader("QuadLights"))
		{
			ImGui::SliderInt("Samples", &qlNumSamples, 1, 32);
			ImGui::Checkbox("1 Sided", &qlOneSided);

			for(int i = 0; i < static_cast<int>(scene.m_quadLights.size()); i++)
			{
				if(ImGui::TreeNode(("QL " + std::to_string(i)).c_str()))
				{
					QuadLight& light = scene.m_quadLights[i];

					ImGui::DragFloat3("Pos", &light.m_quad.m_pos.x, 0.01f);
					ImGui::DragFloat3("Dir", &light.m_quad.m_dir.x, 1.0f);
					mat4 baseMat = mat4::Identity();
					baseMat = baseMat * mat4::RotateX(DEG_TO_RAD(light.m_quad.m_dir.x));
					baseMat = baseMat * mat4::RotateY(DEG_TO_RAD(light.m_quad.m_dir.y));
					baseMat = baseMat * mat4::RotateZ(DEG_TO_RAD(light.m_quad.m_dir.z));
					light.m_quad.T = mat4::Translate(light.m_quad.m_pos) * baseMat;
					light.m_quad.invT = light.m_quad.T.FastInvertedTransformNoScale();

					ImGui::DragFloat("Size", &light.m_quad.size, 0.01f, 0.0f, 100.0f);

					ImGui::ColorEdit3("Color", &light.m_color.x);
					ImGui::DragFloat("Intensity", &light.m_intensity, 0.01f, 0.0f, 1000.0f);

					ImGui::TreePop();
				}
			}
		}
	}

}
