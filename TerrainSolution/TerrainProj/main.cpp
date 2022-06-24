#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>

#include "imgui.h"
#include "imgui-SFML.h"

#include <SFML/Graphics.hpp>

#include "utils/time.h"
#include "utils/Math.h"
#include "utils/RandomGenerator.h"
#include "utils/SimplexNoise.h"
#include "BaseScene.h"

#include "TestScene.h"

void showInfoOverlay(const std::vector<std::string>&);

int main()
{
	auto start = std::chrono::steady_clock::now();

	TestScene scene{ 2048, 1024, "SFML Static" };
	
	ImGui::SFML::Init(scene.getWindow());
	bool showImGuiDemo = false;

	sf::Clock clock;
	while (scene.getWindow().isOpen())
	{
		sf::Event event;
		while (scene.getWindow().pollEvent(event))
		{
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed)
				scene.getWindow().close();

			scene.processEvents(event);
		}

		sf::Time elapsed = clock.restart();
		ImGui::SFML::Update(scene.getWindow(), elapsed);

		showInfoOverlay(scene.getOverlayMessages());
		scene.customImGui();

		if (showImGuiDemo)
			ImGui::ShowDemoWindow(&showImGuiDemo);

		scene.getWindow().clear();

		scene.update(elapsed);

		scene.draw(elapsed);

		ImGui::SFML::Render(scene.getWindow());
		scene.getWindow().display();
	}

	ImGui::SFML::Shutdown();
	std::cerr << "\n\nElapsed(ms)=" << since(start).count() << std::endl;
	return EXIT_SUCCESS;
}

void showInfoOverlay(const std::vector<std::string>& messages)
{
	bool p_open = true;
	static int corner = 1;
	ImGuiIO& io = ImGui::GetIO();
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
	
	if (corner != -1)
	{
		const float PAD = 10.0f;
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
		ImVec2 work_size = viewport->WorkSize;
		ImVec2 window_pos, window_pos_pivot;
		window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
		window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
		window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
		window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
		ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
		window_flags |= ImGuiWindowFlags_NoMove;
	}

	ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background

	if (ImGui::Begin("Info Overlay", &p_open, window_flags))
	{
		ImGui::SetWindowFontScale(1.8f);
		ImGui::Text("Average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		
		ImGui::Separator();

		if (ImGui::IsMousePosValid())
			ImGui::Text("Mouse Position: (%.1f,%.1f)", io.MousePos.x, io.MousePos.y);
		else
			ImGui::Text("Mouse Position: <invalid>");

		if (messages.size() > 0)
		{
			for (auto& msg : messages)
			{
				ImGui::Separator();
				ImGui::Text(msg.c_str());
			}
		}

		if (ImGui::BeginPopupContextWindow())
		{
			if (ImGui::MenuItem("Custom", NULL, corner == -1)) corner = -1;
			if (ImGui::MenuItem("Top-left", NULL, corner == 0)) corner = 0;
			if (ImGui::MenuItem("Top-right", NULL, corner == 1)) corner = 1;
			if (ImGui::MenuItem("Bottom-left", NULL, corner == 2)) corner = 2;
			if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
			if (p_open && ImGui::MenuItem("Close")) p_open = false;
			ImGui::EndPopup();
		}
	}
	ImGui::End();
}