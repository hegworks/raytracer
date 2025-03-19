#include "precomp.h"

#include "renderer.h"


void DecomposeQuaternionToEuler(const quat& q, float3& euler) {
// Extract Euler angles from quaternion
// Note: This uses XYZ rotation order

// Convert quaternion to rotation matrix first
	float xx = q.x * q.x;
	float xy = q.x * q.y;
	float xz = q.x * q.z;
	float xw = q.x * q.w;
	float yy = q.y * q.y;
	float yz = q.y * q.z;
	float yw = q.y * q.w;
	float zz = q.z * q.z;
	float zw = q.z * q.w;

	// Matrix elements
	float m00 = 1 - 2 * (yy + zz);
	float m01 = 2 * (xy - zw);
	float m02 = 2 * (xz + yw);
	float m10 = 2 * (xy + zw);
	float m11 = 1 - 2 * (xx + zz);
	float m12 = 2 * (yz - xw);
	float m20 = 2 * (xz - yw);
	float m21 = 2 * (yz + xw);
	float m22 = 1 - 2 * (xx + yy);

	// Extract angles (in radians)
	// Note: atan2 returns values in [-π, π]
	if(m20 < 0.999999)
	{
		if(m20 > -0.999999)
		{
// Normal case
			euler.x = atan2(-m21, m22);  // X rotation (pitch)
			euler.y = asin(m20);         // Y rotation (yaw)
			euler.z = atan2(-m10, m00);  // Z rotation (roll)
		}
		else
		{
			 // m20 = -1 (north pole singularity)
			euler.x = -atan2(m01, m11);
			euler.y = -PI / 2;
			euler.z = 0;
		}
	}
	else
	{
	 // m20 = 1 (south pole singularity)
		euler.x = atan2(m01, m11);
		euler.y = PI / 2;
		euler.z = 0;
	}

	// Convert to degrees
	euler.x = RAD_TO_DEG(euler.x);
	euler.y = RAD_TO_DEG(euler.y);
	euler.z = RAD_TO_DEG(euler.z);
}

// -----------------------------------------------------------
// Update user interface (imgui)
// -----------------------------------------------------------
void Renderer::UI()
{
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(SCRWIDTH - 300, 0));
	ImGui::SetNextWindowSize(ImVec2(300, SCRHEIGHT));
	ImGui::SetNextWindowBgAlpha(0.2f);
	ImGui::Begin("General", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

	ImGui::Text("avg	fps	rps");
	ImGui::Text("%.1f	%.0f	%.0f", davg, dfps, drps);

	// ray query on mouse
	int2 coord = isDbgPixel ? dbgpixel : mousePos;
	bool isInScreen = coord.x >= 0 && coord.x < SCRWIDTH && coord.y >= 0 && coord.y < SCRHEIGHT;
	uint pixel = 0xFF00FF, red = 0xFFFFFF, green = 0xFFFFFF, blue = 0xFFFFFF;
	if(isInScreen)
	{
		pixel = screen->pixels[coord.x + coord.y * SCRWIDTH];
		red = (pixel & 0xFF0000) >> 16;
		green = (pixel & 0x00FF00) >> 8;
		blue = pixel & 0x0000FF;
	}
	Ray r = camera.GetPrimaryRay((float)coord.x, (float)coord.y, false, 0);
	scene.Intersect(r);
	ImVec4 color = isInScreen ? ImVec4(red / 255.0f, green / 255.0f, blue / 255.0f, 1.0f) : ImVec4(1.0f, 0.0f, 1.0f, 1.0f);
	int debugInt = r.hit.t < BVH_FAR ? scene.m_blasList[r.hit.inst].blasIdx : -1;
	ImGui::Text("%i  %i,%i", debugInt, coord.x, coord.y);
	ImGui::SameLine();
	ImGui::ColorButton("", color);
	ImGui::SameLine();
	ImGui::Text("%u,%u,%u", red, green, blue);

	if(ImGui::BeginTabBar("Main"))
	{
		if(ImGui::BeginTabItem("General"))
		{
			if(ImGui::Button("ResetCam")) tddResetCam = true;
			ImGui::SameLine();
			ImGui::Checkbox("Fixed Seed", &isDbgFixSeed);

			//ImGui::Checkbox("Animate", &animating);
			ImGui::Checkbox("AA", &useAA);
			ImGui::SameLine();
			ImGui::Checkbox("ACM", &useACM);
			ImGui::SameLine();
			ImGui::Checkbox("TDD", &tdd);

			ImGui::Separator();

			ImGui::Checkbox("Skydome", &useSD);
			ImGui::SliderFloat("Brightness", &dbgSDBF, 0.0f, 5.0f);

			ImGui::Separator();

			ImGui::Checkbox("Depth of Field", &useDOF);
			ImGui::DragFloat("DOF angle", &defocusAngle, 0.1f, EPS, 40);
			ImGui::DragFloat("DOF distance", &focusDistance, 0.1f, EPS, 9999);

			ImGui::Separator();

			ImGui::Checkbox("Stochastic Lights", &dbgSL);
			ImGui::SliderInt("Samples", &dbgSLS, 1, 300);

			ImGui::Separator();

			ImGui::Checkbox("Stochastic Fresnel", &dbgSF);
			ImGui::SliderInt("ndal", &ndal, 0, 3);
			ImGui::SliderInt("SPP", &spp, 1, 30);
			ImGui::SliderInt("Depth", &maxDepth, 1, 50);
			ImGui::SliderFloat("FireFly", &dbgFF, 0.0f, 40.0f);
			ImGui::SliderFloat("Gamma", &dbgGC, 0.01f, 5.0f);

			const char* epsTypes[] =
			{
				"1e-1",
				"1e-2",
				"1e-3",
				"1e-4",
				"1e-5",
				"1e-6",
				"1e-7",
			};
			if(ImGui::Combo("EPS", &epsInt, epsTypes, IM_ARRAYSIZE(epsTypes)))
			{
				EPS = 1.0f;
				for(int i = 0; i < epsInt; ++i)
				{
					EPS *= 0.1f;
				}
			}

			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Debugger"))
		{
			if(ImGui::Button("ResetCam"))
			{
				tddResetCam = true;
			}
			ImGui::SameLine();
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
				ImU32 imColor = IM_COL32(255, 0, 255, 255);

				float size = 20.0f; // Cursor size (change as needed)
				ImVec2 pos = {static_cast<float>(dbgpixel.x),static_cast<float>(dbgpixel.y)};
				drawList->AddRectFilled(pos, ImVec2(pos.x + size, pos.y + size), imColor);
			}

			if(ImGui::SliderInt2("SCR RANGE X", &dbgScrRangeX.x, 0, SCRWIDTH) ||
			   ImGui::SliderInt2("SCR RANGE Y", &dbgScrRangeY.x, 0, SCRHEIGHT))
			{
				screen->Clear(0);
			}

			ImGui::EndTabItem();
		}
		if(ImGui::BeginTabItem("Lights"))
		{
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

			if(!scene.m_pointLightList.empty())
			{
				if(ImGui::CollapsingHeader("PointLights"))
				{
					for(int i = 0; i < static_cast<int>(scene.m_pointLightList.size()); i++)
					{
						if(ImGui::TreeNode(("PL " + std::to_string(i)).c_str()))
						{
							ImGui::DragFloat3("Pos", &scene.m_pointLightList[i].m_pos.x, 0.01f);
							ImGui::ColorEdit3("Color", &scene.m_pointLightList[i].m_color.x);
							ImGui::DragFloat("Intensity", &scene.m_pointLightList[i].m_intensity, 0.01f, 0.0f, 1000.0f);

							ImGui::TreePop();
						}
					}
				}
			}
			if(!scene.m_spotLightList.empty())
			{
				if(ImGui::CollapsingHeader("SpotLights"))
				{
					for(int i = 0; i < static_cast<int>(scene.m_spotLightList.size()); i++)
					{
						if(ImGui::TreeNode(("SL " + std::to_string(i)).c_str()))
						{
							ImGui::DragFloat3("Pos", &scene.m_spotLightList[i].m_pos.x, 0.01f);
							ImGui::ColorEdit3("Color", &scene.m_spotLightList[i].m_color.x);
							ImGui::DragFloat("Intensity", &scene.m_spotLightList[i].m_intensity, 0.01f, 0.0f, 1000.0f);
							float3 dir = scene.m_spotLightList[i].m_dir;
							ImGui::DragFloat3("Dir", &dir.x, 0.001f, -1.0f, 1.0f);
							scene.m_spotLightList[i].m_dir = normalize(dir);

							ImGui::DragFloat("CosI", &scene.m_spotLightList[i].m_cosI, 0.001f, scene.m_spotLightList[i].m_cosO, 1.0f);
							ImGui::SameLine();
							ImGui::Text("%.2f", RAD_TO_DEG(acos(scene.m_spotLightList[i].m_cosI)));

							ImGui::DragFloat("CosO", &scene.m_spotLightList[i].m_cosO, 0.001f, 0.0f, scene.m_spotLightList[i].m_cosI);
							ImGui::SameLine();
							ImGui::Text("%.2f", RAD_TO_DEG(acos(scene.m_spotLightList[i].m_cosO)));

							ImGui::TreePop();
						}
					}
				}
			}
			if(!scene.m_dirLightList.empty())
			{
				if(ImGui::CollapsingHeader("DirLights"))
				{
					for(int i = 0; i < static_cast<int>(scene.m_dirLightList.size()); i++)
					{
						if(ImGui::TreeNode(("DL " + std::to_string(i)).c_str()))
						{
							DirLight& light = scene.m_dirLightList[i];
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
			if(!scene.m_quadLightList.empty())
			{
				if(ImGui::CollapsingHeader("QuadLights"))
				{
					ImGui::SliderInt("Samples", &qlNumSamples, 1, 32);
					ImGui::Checkbox("1 Sided", &qlOneSided);

					for(int i = 0; i < static_cast<int>(scene.m_quadLightList.size()); i++)
					{
						if(ImGui::TreeNode(("QL " + std::to_string(i)).c_str()))
						{
							QuadLight& light = scene.m_quadLightList[i];

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
			ImGui::EndTabItem();
		}

		if(ImGui::BeginTabItem("Objects"))
		{
			for(int i = 0; i < NUM_MODEL_TYPES; ++i)
			{
				if(ImGui::Button(("+" + ALL_MODEL_NAMES[i]).c_str()))
				{
					scene.CreateModel(ALL_MODEL_TYPES[i]);
				}
				if(i % 3 < 2 && i != NUM_MODEL_TYPES - 1)
				{
					ImGui::SameLine();
				}
			}

			ImGui::Separator();

			int numModels = static_cast<int>(scene.m_modelList.size());
			int numBlases = static_cast<int>(scene.m_blasList.size());
			ImGui::Text("NumModels: %i", numModels);
			ImGui::Text("NumBlases: %i", numBlases);

			if(ImGui::BeginTabBar("Objects"))
			{
				if(ImGui::BeginTabItem("Materials"))
				{
					for(int i = 0; i < numModels; ++i)
					{
						Model& model = scene.m_modelList[i];
						if(ImGui::TreeNode((std::to_string(i) + " " + model.m_fileName).c_str()))
						{
							for(int j = 0; j < model.m_modelData.m_meshMaterialList.size(); j++)
							{
								if(j > 0) ImGui::Separator();
								Material& mat = model.m_modelData.m_meshMaterialList[j];
								if(mat.m_name[0] != '\0') ImGui::Text(mat.m_name);
								int matInt = static_cast<int>(mat.m_type);
								ImGui::Combo(("Type##" + std::to_string(j)).c_str(), &matInt, MATERIAL_STRING, IM_ARRAYSIZE(MATERIAL_STRING));
								mat.m_type = static_cast<Material::Type>(matInt);
								ImGui::ColorEdit3(("Albedo##" + std::to_string(j)).c_str(), &mat.m_albedo.x);

								switch(mat.m_type)
								{
									case Material::Type::DIFFUSE:
									case Material::Type::GLOSSY:
										break;
									case Material::Type::REFRACTIVE:
										ImGui::DragFloat(("Density##" + std::to_string(j)).c_str(), &mat.m_factor0, 0.01f, 0.0f, 999999.0f);
										ImGui::DragFloat(("IOR##" + std::to_string(j)).c_str(), &mat.m_factor1, 0.01f, 0.0f, 999999.0f);
										break;
									case Material::Type::PATH_TRACED:
										ImGui::DragFloat(("Smoothness##" + std::to_string(j)).c_str(), &mat.m_factor0, 0.001f, 0, 1);
										ImGui::DragFloat(("Specularity##" + std::to_string(j)).c_str(), &mat.m_factor1, 0.001f, 0, 1);
										break;
									case Material::Type::EMISSIVE:
										ImGui::DragFloat(("Intensity##" + std::to_string(j)).c_str(), &mat.m_factor0, 0.01f, 0.0f, 999999.0f);
										break;
								}
							}
							ImGui::TreePop();
						}
					}
					ImGui::EndTabItem();
				}
				if(ImGui::BeginTabItem("Transforms"))
				{
					ImGui::Text("Rotation Axis:");
					ImGui::SameLine();
					if(ImGui::RadioButton("X", &dbgRotAxisInt, 0)) { rotAxis = float3(1, 0, 0); }
					ImGui::SameLine();
					if(ImGui::RadioButton("Y", &dbgRotAxisInt, 1)) { rotAxis = float3(0, 1, 0); }
					ImGui::SameLine();
					if(ImGui::RadioButton("Z", &dbgRotAxisInt, 2)) { rotAxis = float3(0, 0, 1); }

					for(int i = 0; i < numBlases; ++i)
					{
						tinybvh::BLASInstance& blas = scene.m_blasList[i];
						Model& model = scene.m_modelList[blas.blasIdx];
						if(ImGui::TreeNode((model.m_fileName + " " + std::to_string(i)).c_str()))
						{
							Transform& t = scene.m_tranformList[i];
							bool changed = false;

							if(ImGui::DragFloat3("Pos", &t.m_pos.x, 0.1f)) changed = true;
							ImGui::SameLine();
							if(ImGui::Button("reset##0")) t.m_pos = 0, changed = true;


#if 0
							// Store the previous angles to calculate deltas
							static float3 prevAngles = t.m_rotAngles;

							bool rotChanged = false;
							if(ImGui::DragFloat("Rotation X", &t.m_rotAngles.x, 0.5f)) rotChanged = true;
							if(ImGui::DragFloat("Rotation Y", &t.m_rotAngles.y, 0.5f)) rotChanged = true;
							if(ImGui::DragFloat("Rotation Z", &t.m_rotAngles.z, 0.5f)) rotChanged = true;

							if(rotChanged)
							{
								// Calculate angle deltas
								float deltaX = t.m_rotAngles.x - prevAngles.x;
								float deltaY = t.m_rotAngles.y - prevAngles.y;
								float deltaZ = t.m_rotAngles.z - prevAngles.z;

								// Create incremental rotations based on local axes
								if(deltaX != 0.0f)
								{
				// Create rotation around local X axis
									float3 localXAxis = t.m_rot.rotateVector({1, 0, 0});
									quat incRotX = quat::FromAxisAngle(localXAxis, DEG_TO_RAD(deltaX));
									t.m_rot = incRotX * t.m_rot;
								}

								if(deltaY != 0.0f)
								{
				// Create rotation around local Y axis
									float3 localYAxis = t.m_rot.rotateVector({0, 1, 0});
									quat incRotY = quat::FromAxisAngle(localYAxis, DEG_TO_RAD(deltaY));
									t.m_rot = incRotY * t.m_rot;
								}

								if(deltaZ != 0.0f)
								{
				// Create rotation around local Z axis
									float3 localZAxis = t.m_rot.rotateVector({0, 0, 1});
									quat incRotZ = quat::FromAxisAngle(localZAxis, DEG_TO_RAD(deltaZ));
									t.m_rot = incRotZ * t.m_rot;
								}

								// Update previous angles
								prevAngles = t.m_rotAngles;
								changed = true;
							}
#endif


							bool rotChanged = false;
							float oldX = t.m_rotAngles.x;
							float oldY = t.m_rotAngles.y;
							float oldZ = t.m_rotAngles.z;

							if(ImGui::DragFloat("Rotation X", &t.m_rotAngles.x, 0.5f)) rotChanged = true;
							if(ImGui::DragFloat("Rotation Y", &t.m_rotAngles.y, 0.5f)) rotChanged = true;
							if(ImGui::DragFloat("Rotation Z", &t.m_rotAngles.z, 0.5f)) rotChanged = true;

							if(rotChanged)
							{
				// Calculate deltas from UI
								float deltaX = t.m_rotAngles.x - oldX;
								float deltaY = t.m_rotAngles.y - oldY;
								float deltaZ = t.m_rotAngles.z - oldZ;

								// Apply incremental rotations around global axes
								if(deltaX != 0.0f)
								{
									quat incRotX = quat::FromAxisAngle({1, 0, 0}, DEG_TO_RAD(deltaX));
									t.m_rot = t.m_rot * incRotX;
								}

								if(deltaY != 0.0f)
								{
									quat incRotY = quat::FromAxisAngle({0, 1, 0}, DEG_TO_RAD(deltaY));
									t.m_rot = t.m_rot * incRotY;
								}

								if(deltaZ != 0.0f)
								{
									quat incRotZ = quat::FromAxisAngle({0, 0, 1}, DEG_TO_RAD(deltaZ));
									t.m_rot = t.m_rot * incRotZ;
								}

								// Now decompose the final quaternion back to Euler angles
								// This updates the UI to match the actual rotation
								DecomposeQuaternionToEuler(t.m_rot, t.m_rotAngles);

								changed = true;
							}

							ImGui::SameLine();
							if(ImGui::Button("reset##1")) t.m_rot = quat::identity(), changed = true;

							bool uniformScaleChanged = false;
							if(ImGui::DragFloat3("Scl", &t.m_scl.x, 0.1f, EPS, 99999.0f)) changed = true;
							if(t.m_scl.x < EPS) t.m_scl.x = EPS;
							if(t.m_scl.y < EPS) t.m_scl.y = EPS;
							if(t.m_scl.z < EPS) t.m_scl.z = EPS;
							ImGui::SameLine();
							if(ImGui::Button("0##0")) t.m_scl = EPS, changed = true;
							ImGui::SameLine();
							if(ImGui::Button("1##0")) t.m_scl = 1, changed = true;
							float uniformScale = t.m_scl.x;
							if(ImGui::DragFloat("Uni.Scl", &uniformScale, 0.1f, EPS, 99999.0f)) changed = true, uniformScaleChanged = true;
							if(uniformScaleChanged)
							{
								if(uniformScale < EPS) uniformScale = EPS;
								t.m_scl = uniformScale;
							}

							if(changed)
							{
								scene.SetBlasTransform(blas, t);
								scene.BuildTlas();
							}
							ImGui::TreePop();
						}
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::End();

	if(tdd)
	{
		ImGui::SetNextWindowPos(ImVec2(0, 0));
		ImGui::SetNextWindowSize(ImVec2(300, SCRHEIGHT / 2.0f));
		ImGui::SetNextWindowBgAlpha(0.2f);
		ImGui::Begin("2D Debugger", nullptr, ImGuiWindowFlags_NoResize);

		if(ImGui::Button("Reset"))
		{
			tddSceneScale = 2.0f;
			tddOffset = tddOffset = int2(0, 0);
			tddResetCam = true;
		}
		ImGui::SameLine();
		ImGui::Checkbox("Black Background", &tddBBG);

		ImGui::DragFloat("Scale", &tddSceneScale, 0.01f, 0.001f, 10.0f);
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
