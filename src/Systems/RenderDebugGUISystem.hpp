#pragma once

#include "../Components/TransformComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <SDL2/SDL.h>
#include <flecs.h>

class RenderDebugGUISystem
{
public:
	RenderDebugGUISystem(flecs::world& ecs)
	{
		ecs.system<>()
			.each([this](flecs::entity e) {
				Update(e.world().get<SDL_Renderer>(), e.world(), e.world().get<SDL_Rect>());
			});
	}

	void Update(SDL_Renderer* Renderer, flecs::world& world, const SDL_Rect& Camera)
	{
		ImGui_ImplSDLRenderer2_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Begin("Spawn Enemies"))
		{
			static int PositionX = 0;
			static int PositionY = 0;
			static int ScaleX = 1;
			static int ScaleY = 1;
			static int VelocityX = 0;
			static int VelocityY = 0;
			static int Health = 100;
			static float Rotation = 0.0;
			static float ProjectileAngle = 0.0;
			static float ProjectileSpeed = 100.0;
			static int ProjectileRepeat = 10;
			static int ProjectileDuration = 10;
			const char* Sprites[] = { "tank-image", "truck-image" };
			static int SelectedSpriteIndex = 0;

			// Section to input enemy sprite texture ID
			if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::Combo("Texture ID", &SelectedSpriteIndex, Sprites, IM_ARRAYSIZE(Sprites));
			}
			ImGui::Spacing();

			// Section to input enemy transform component
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::InputInt("Position X", &PositionX);
				ImGui::InputInt("Position Y", &PositionY);
				ImGui::SliderInt("Scale X", &ScaleX, 1, 10);
				ImGui::SliderInt("Scale Y", &ScaleY, 1, 10);
				ImGui::SliderAngle("Rotation (degrees)", &Rotation, 0.0f, 360.0f);
			}
			ImGui::Spacing();

			// Section to input enemy rigid body component
			if (ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::InputInt("Velocity X", &VelocityX);
				ImGui::InputInt("Velocity Y", &VelocityY);
			}
			ImGui::Spacing();

			// Section to input enemy projectile emitter component
			if (ImGui::CollapsingHeader("Projectile Emitter", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderAngle("Projectile Angle (degrees)", &ProjectileAngle, 0.0f, 360.0f);
				ImGui::InputFloat("Projectile Speed (px / sec)", &ProjectileSpeed, 10, 500);
				ImGui::InputInt("Projectile Repeat", &ProjectileRepeat);
				ImGui::InputInt("Projectile Duration", &ProjectileDuration);
			}
			ImGui::Spacing();

			// Section to input enemy health component
			if (ImGui::CollapsingHeader("Health", ImGuiTreeNodeFlags_DefaultOpen))
			{
				ImGui::SliderInt("%", &Health, 0, 100);
			}
			ImGui::Spacing();

			ImGui::Separator();
			ImGui::Spacing();

			// Button to spawn an enemy
			if (ImGui::Button("Spawn enemy"))
			{
				flecs::entity Enemy = world.entity();
				Enemy.add<TransformComponent>()
					.set<TransformComponent>({glm::vec2(PositionX, PositionY), glm::vec2(ScaleX, ScaleY), glm::degrees(Rotation)});
				Enemy.add<RigidBodyComponent>()
					.set<RigidBodyComponent>({glm::vec2(VelocityX, VelocityY)});
				Enemy.add<SpriteComponent>()
					.set<SpriteComponent>({Sprites[SelectedSpriteIndex], 32, 32, 1});
				Enemy.add<BoxColliderComponent>()
					.set<BoxColliderComponent>({25, 20, glm::vec2(5, 5)});
				double ProjectileVelocityX = ProjectileSpeed * cos(ProjectileAngle);
				double ProjectileVelocityY = ProjectileSpeed * sin(ProjectileAngle);
				Enemy.add<ProjectileEmitterComponent>()
					.set<ProjectileEmitterComponent>({glm::vec2(ProjectileVelocityX, ProjectileVelocityY), ProjectileRepeat * 1000, ProjectileDuration * 1000, 10, false});
				Enemy.add<HealthComponent>()
					.set<HealthComponent>({Health});

				// Reset all input values after we create the enemy
				PositionX = PositionY = Rotation = ProjectileAngle = 0;
				ScaleX = ScaleY = 1;
				ProjectileRepeat = ProjectileDuration = 10;
				ProjectileSpeed = Health = 100;
			}
		}
		ImGui::End();

		// Display a small overlay window with the map positions of the mouse
		ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
		ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0, 0));
		ImGui::SetNextWindowBgAlpha(0.9f);
		if (ImGui::Begin("Map coordinates", nullptr, WindowFlags))
		{
			ImGui::Text("Map coordinates: (x=%.1f, y=%.1f)", ImGui::GetIO().MousePos.x + Camera.x, ImGui::GetIO().MousePos.y + Camera.y);
		}
		ImGui::End();

		ImGui::Render();
		ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), Renderer);
	}
};
