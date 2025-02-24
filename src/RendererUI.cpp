#include "precomp.h"

#include "renderer.h"

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::SetWindowPos(ImVec2(SCRWIDTH - 300, 0));
	ImGui::SetWindowSize(ImVec2(300, SCRHEIGHT));

	ImGui::Text("avg	fps	rps");
	ImGui::Text("%.1f	%.0f	%.0f", davg, dfps, drps);

	if(ImGui::Button("ResetCam"))
	{
		tddResetCam = true;
	}

	ImGui::Checkbox("Fixed Seed", &isDbgFixSeed);
	if(!isDbgPixel)
	{
		ImGui::SameLine();
		if(ImGui::Button("DBG Pixel"))
		{
			isDbgPixel = true;
		}
	}
	else if(isDbgPixelClicked && !isDbgPixelEntered)
	{
		ImDrawList* drawList = ImGui::GetForegroundDrawList();
		ImU32 color = IM_COL32(255, 0, 255, 255);

		float size = 20.0f; // Cursor size (change as needed)
		ImVec2 pos = {static_cast<float>(dbgpixel.x),static_cast<float>(dbgpixel.y)};
		drawList->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), color);

		//drawList->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + 1, pos.y + 1), color);

	}

	ImGui::Separator();
	if(ImGui::SliderInt2("SCR RANGE X", &dbgScrRangeX.x, 0, SCRWIDTH) ||
	   ImGui::SliderInt2("SCR RANGE Y", &dbgScrRangeY.x, 0, SCRHEIGHT))
	{
		screen->Clear(0);
	}


	ImGui::Separator();

	ImGui::Checkbox("Animate", &animating);
	ImGui::SameLine();
	ImGui::Checkbox("AA", &useAA);
	ImGui::SameLine();
	ImGui::Checkbox("TDD", &tdd);

	// ray query on mouse
	int2 coord = isDbgPixel ? dbgpixel : mousePos;
	Ray r = camera.GetPrimaryRay((float)coord.x, (float)coord.y);
	scene.FindNearest(r);
	bool isInScreen = coord.x >= 0 && coord.x < SCRWIDTH && coord.y >= 0 && coord.y < SCRHEIGHT;
	uint pixel = 0xFF00FF, red = 0xFFFFFF, green = 0xFFFFFF, blue = 0xFFFFFF;
	if(isInScreen)
	{
		pixel = screen->pixels[coord.x + coord.y * SCRWIDTH];
		red = (pixel & 0xFF0000) >> 16;
		green = (pixel & 0x00FF00) >> 8;
		blue = pixel & 0x0000FF;
	}
	ImVec4 color = isInScreen ? ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f) : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	ImGui::ColorButton("", color);
	ImGui::SameLine();

	ImGui::Text("%u,%u,%u  %i,%i  %i", red, green, blue, coord.x, coord.y, r.objIdx);

	ImGui::SliderInt("ndal", &ndal, 0, 3);

	ImGui::Checkbox("ACM MAX", &useACMMax);
	ImGui::SameLine();
	ImGui::SliderInt(" ", &acmMax, 1, 1000);

	ImGui::SliderInt("Depth", &maxDepth, 1, 20);

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
	ImGui::End();


	if(tdd)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 360));
		ImGui::SetNextWindowSize(ImVec2(300, SCRHEIGHT / 2.0f));
		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::Begin("2D Debugger", nullptr, ImGuiWindowFlags_NoResize);

		if(ImGui::Button("Reset"))
		{
			tddSceneScale = 1.3f;
			tddOffset = tddOffset = int2(0, 130);
		}
		ImGui::SameLine();
		ImGui::Checkbox("Black Background", &tddBBG);

		ImGui::DragFloat("Scale", &tddSceneScale, 0.01f, 0.001f, 2.5f);
		ImGui::DragInt2("Offset", &tddOffset.x);
		ImGui::DragInt("Ray Count", &tddrx, 0.5f, 1, 200);
		ImGui::SliderInt("Font Size", &tddFS, 0, 4);

		ImGui::Separator();

		ImGui::Checkbox("Single", &tddSXM);
		ImGui::SameLine();
		ImGui::DragInt("X", &tddSXX, 1.0f, 0, SCRWIDTH);

		ImGui::Separator();

		ImGui::Text("Ray");
		ImGui::SameLine();
		ImGui::Checkbox("##0", &tddPRay);
		ImGui::SameLine();
		ImGui::Checkbox("Length##0", &tddPRayL);
		ImGui::SameLine();
		ImGui::Checkbox("Coord##0", &tddRC);

		ImGui::Text("Normal");
		ImGui::SameLine();
		ImGui::Checkbox("##1", &tddPN);
		ImGui::SameLine();
		ImGui::Checkbox("Length##1", &tddPNL);

		ImGui::Text("PointLight");
		ImGui::SameLine();
		ImGui::Checkbox("Pos", &tddPLP);
		ImGui::SameLine();
		ImGui::Checkbox("Ray", &tddPLR);

		ImGui::End();
	}

	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(300, SCRHEIGHT / 2.0f));
		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::Begin("Materials", nullptr, ImGuiWindowFlags_NoResize);

		ImGui::DragFloat("ior", &dbgIor, 0.01f, 1.0, 10.0);
		ImGui::SameLine();
		ImGui::Checkbox("Beer", &dbgBeer);

		const char* materialTypes[] =
		{
			"DIFFUSE",
			"REFLECTIVE",
			"GLOSSY",
			"REFRACTIVE"
		};

		if(ImGui::CollapsingHeader("Sphere"))
		{
			Material& mat = scene.sphere.m_material;
			int matInt = static_cast<int>(mat.m_type);
			ImGui::Combo("Type##1", &matInt, materialTypes, IM_ARRAYSIZE(materialTypes));
			mat.m_type = static_cast<Material::Type>(matInt);
			ImGui::ColorEdit3("Albedo##1", &mat.m_albedo.x);
			ImGui::DragFloat("Glossiness##1", &mat.m_glossiness, 0.01f, 0.0f, 30.0f);
		}
		if(ImGui::CollapsingHeader("Torus"))
		{
			Material& mat = scene.torus.m_material;
			int matInt = static_cast<int>(mat.m_type);
			ImGui::Combo("Type##2", &matInt, materialTypes, IM_ARRAYSIZE(materialTypes));
			mat.m_type = static_cast<Material::Type>(matInt);
			ImGui::ColorEdit3("Albedo##2", &mat.m_albedo.x);
			ImGui::DragFloat("Glossiness##2", &mat.m_glossiness, 0.01f, 0.0f, 30.0f);
		}
		if(ImGui::CollapsingHeader("Cube"))
		{
			Material& mat = scene.cube.m_material;
			int matInt = static_cast<int>(mat.m_type);
			ImGui::Combo("Type##3", &matInt, materialTypes, IM_ARRAYSIZE(materialTypes));
			mat.m_type = static_cast<Material::Type>(matInt);
			ImGui::ColorEdit3("Albedo##3", &mat.m_albedo.x);
			ImGui::DragFloat("Glossiness##3", &mat.m_glossiness, 0.01f, 0.0f, 30.0f);
		}

		ImGui::End();
	}
}

void Renderer::MouseDown(int button)
{
	if(isDbgPixel && !isDbgPixelClicked)
	{
		if(button == GLFW_MOUSE_BUTTON_LEFT)
		{
			isDbgPixelClicked = true;
		}
	}
}

void Renderer::KeyDown(int key)
{
	if(isDbgPixel && isDbgPixelClicked)
	{
		if(key == GLFW_KEY_UP) dbgpixel.y = dbgpixel.y == 0 ? 0 : dbgpixel.y - 1;
		if(key == GLFW_KEY_DOWN) dbgpixel.y = dbgpixel.y == SCRHEIGHT - 1 ? SCRHEIGHT - 1 : dbgpixel.y + 1;
		if(key == GLFW_KEY_LEFT) dbgpixel.x = dbgpixel.x == 0 ? 0 : dbgpixel.x - 1;
		if(key == GLFW_KEY_RIGHT) dbgpixel.x = dbgpixel.x == SCRWIDTH - 1 ? SCRWIDTH - 1 : dbgpixel.x + 1;

		if(key == GLFW_KEY_ENTER)
		{
			printf("DBG PIXEL AT %i,%i\n", dbgpixel.x, dbgpixel.y);
			isDbgPixelEntered = true;
		}
	}
}
