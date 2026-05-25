#include "FlecsSystems.hpp"
#include "../AssetManager/AssetManager.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TextLabelComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlrenderer3.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <string>
#include <vector>

static void RenderBeginSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	auto& Context = World.get_mut<GameContext>();
	if (!Context.Renderer)
	{
		return;
	}

	SDL_SetRenderDrawColor(Context.Renderer, 21, 21, 21, 255);
	SDL_RenderClear(Context.Renderer);
}

static void RenderSpriteSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	struct RenderableEntity
	{
		TransformComponent Transform;
		SpriteComponent Sprite;
	};

	auto& Context = World.get_mut<GameContext>();
	if (!Context.Renderer || !Context.Assets || !Context.Camera)
	{
		return;
	}

	const SDL_FRect& Camera = *Context.Camera;
	std::vector<RenderableEntity> RenderableEntities;
	World.each([&RenderableEntities, &Camera](const TransformComponent& Transform, const SpriteComponent& Sprite)
	{
		const bool IsOutsideCameraView =
			Transform.Position.x + (Transform.Scale.x * Sprite.Width) < Camera.x ||
			Transform.Position.x > Camera.x + Camera.w ||
			Transform.Position.y + (Transform.Scale.y * Sprite.Height) < Camera.y ||
			Transform.Position.y > Camera.y + Camera.h;

		if (!IsOutsideCameraView || Sprite.IsFixed)
		{
			RenderableEntities.push_back({ Transform, Sprite });
		}
	});

	std::sort(RenderableEntities.begin(), RenderableEntities.end(), [](const RenderableEntity& A, const RenderableEntity& B)
	{
		return A.Sprite.ZIndex < B.Sprite.ZIndex;
	});

	for (const auto& Renderable : RenderableEntities)
	{
		SDL_FRect SourceRectangle = Renderable.Sprite.SrcRect;
		SDL_FRect DestinationRectangle =
		{
			std::round(static_cast<float>(Renderable.Transform.Position.x - (Renderable.Sprite.IsFixed ? 0.0f : Camera.x))),
			std::round(static_cast<float>(Renderable.Transform.Position.y - (Renderable.Sprite.IsFixed ? 0.0f : Camera.y))),
			std::ceil(static_cast<float>(Renderable.Sprite.Width * Renderable.Transform.Scale.x)),
			std::ceil(static_cast<float>(Renderable.Sprite.Height * Renderable.Transform.Scale.y))
		};

		SDL_RenderTextureRotated
		(
			Context.Renderer,
			Context.Assets->GetTexture(Renderable.Sprite.AssetID),
			&SourceRectangle,
			&DestinationRectangle,
			Renderable.Transform.Rotation,
			nullptr,
			Renderable.Sprite.Flip
		);
	}
}

static void RenderTextSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	auto& Context = World.get_mut<GameContext>();
	if (!Context.Renderer || !Context.Assets || !Context.Camera)
	{
		return;
	}

	const SDL_FRect& Camera = *Context.Camera;
	World.each([&Context, &Camera](const TextLabelComponent& TextLabel)
	{
		SDL_Surface* TextSurface = TTF_RenderText_Blended(Context.Assets->GetFont(TextLabel.AssetID), TextLabel.Text.c_str(), 0, TextLabel.Color);
		SDL_Texture* TextTexture = SDL_CreateTextureFromSurface(Context.Renderer, TextSurface);
		SDL_DestroySurface(TextSurface);

		float LabelWidth = 0.0f;
		float LabelHeight = 0.0f;
		SDL_GetTextureSize(TextTexture, &LabelWidth, &LabelHeight);
		SDL_FRect DestinationRectangle =
		{
			static_cast<float>(TextLabel.Position.x - (TextLabel.IsFixed ? 0.0f : Camera.x)),
			static_cast<float>(TextLabel.Position.y - (TextLabel.IsFixed ? 0.0f : Camera.y)),
			LabelWidth,
			LabelHeight
		};

		SDL_RenderTexture(Context.Renderer, TextTexture, nullptr, &DestinationRectangle);
		SDL_DestroyTexture(TextTexture);
	});
}

static void RenderHealthBarSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	auto& Context = World.get_mut<GameContext>();
	if (!Context.Renderer || !Context.Assets || !Context.Camera)
	{
		return;
	}

	const SDL_FRect& Camera = *Context.Camera;
	World.each([&Context, &Camera](const TransformComponent& Transform, const HealthComponent& Health, const SpriteComponent& Sprite)
	{
		SDL_Color HealthBarColor = { 255, 255, 255 };
		if (Health.HealthPercentage < 40)
		{
			HealthBarColor = { 255, 0, 0 };
		}
		else if (Health.HealthPercentage < 80)
		{
			HealthBarColor = { 255, 255, 0 };
		}
		else
		{
			HealthBarColor = { 0, 255, 0 };
		}

		const float HealthBarWidth = 15.0f;
		const float HealthBarHeight = 3.0f;
		const float HealthBarPositionX = Transform.Position.x + (Sprite.Width * Transform.Scale.x) - Camera.x;
		const float HealthBarPositionY = Transform.Position.y - Camera.y;
		SDL_FRect HealthBarRect =
		{
			HealthBarPositionX,
			HealthBarPositionY,
			HealthBarWidth * (Health.HealthPercentage / 100.0f),
			HealthBarHeight
		};

		SDL_SetRenderDrawColor(Context.Renderer, HealthBarColor.r, HealthBarColor.g, HealthBarColor.b, 255);
		SDL_RenderFillRect(Context.Renderer, &HealthBarRect);

		const std::string HealthText = std::to_string(Health.HealthPercentage) + "%";
		SDL_Surface* TextSurface = TTF_RenderText_Blended(Context.Assets->GetFont("pico8-font-5"), HealthText.c_str(), 0, HealthBarColor);
		SDL_Texture* TextTexture = SDL_CreateTextureFromSurface(Context.Renderer, TextSurface);
		SDL_DestroySurface(TextSurface);

		float LabelWidth = 0.0f;
		float LabelHeight = 0.0f;
		SDL_GetTextureSize(TextTexture, &LabelWidth, &LabelHeight);
		SDL_FRect HealthBarTextRectangle = { HealthBarPositionX, HealthBarPositionY + 5.0f, LabelWidth, LabelHeight };
		SDL_RenderTexture(Context.Renderer, TextTexture, nullptr, &HealthBarTextRectangle);
		SDL_DestroyTexture(TextTexture);
	});
}

static void RenderEndSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	auto& Context = World.get_mut<GameContext>();
	if (Context.Renderer)
	{
		SDL_RenderPresent(Context.Renderer);
	}
}

static void RunRenderDebugSystem(flecs::world World)
{
	auto& Context = World.get_mut<GameContext>();
	if (!Context.Renderer || !Context.Camera || !Context.IsDebug || !*Context.IsDebug)
	{
		return;
	}

	const SDL_FRect& Camera = *Context.Camera;
	World.each([&Context, &Camera](const TransformComponent& Transform, const BoxColliderComponent& Collider)
	{
		SDL_FRect ColliderRect =
		{
			static_cast<float>(Transform.Position.x + Collider.Offset.x - Camera.x),
			static_cast<float>(Transform.Position.y + Collider.Offset.y - Camera.y),
			static_cast<float>(Collider.Width * Transform.Scale.x),
			static_cast<float>(Collider.Height * Transform.Scale.y)
		};

		SDL_SetRenderDrawColor(Context.Renderer, 255, 0, 0, 255);
		SDL_RenderRect(Context.Renderer, &ColliderRect);
	});

	ImGui_ImplSDLRenderer3_NewFrame();
	ImGui_ImplSDL3_NewFrame();
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
		static float Rotation = 0.0f;
		static float ProjectileAngle = 0.0f;
		static float ProjectileSpeed = 100.0f;
		static int ProjectileRepeat = 10;
		static int ProjectileDuration = 10;
		const char* Sprites[] = { "tank-tiger-right-texture", "truck-ford-right-texture" };
		static int SelectedSpriteIndex = 0;

		if (ImGui::CollapsingHeader("Sprite", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Combo("Texture ID", &SelectedSpriteIndex, Sprites, IM_ARRAYSIZE(Sprites));
		}

		ImGui::Spacing();
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputInt("Position X", &PositionX);
			ImGui::InputInt("Position Y", &PositionY);
			ImGui::SliderInt("Scale X", &ScaleX, 1, 10);
			ImGui::SliderInt("Scale Y", &ScaleY, 1, 10);
			ImGui::SliderAngle("Rotation (degrees)", &Rotation, 0.0f, 360.0f);
		}

		ImGui::Spacing();
		if (ImGui::CollapsingHeader("Rigid Body", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::InputInt("Velocity X", &VelocityX);
			ImGui::InputInt("Velocity Y", &VelocityY);
		}

		ImGui::Spacing();
		if (ImGui::CollapsingHeader("Projectile Emitter", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderAngle("Projectile Angle (degrees)", &ProjectileAngle, 0.0f, 360.0f);
			ImGui::InputFloat("Projectile Speed (px / sec)", &ProjectileSpeed, 10.0f, 500.0f);
			ImGui::InputInt("Projectile Repeat", &ProjectileRepeat);
			ImGui::InputInt("Projectile Duration", &ProjectileDuration);
		}

		ImGui::Spacing();
		if (ImGui::CollapsingHeader("Health", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::SliderInt("%", &Health, 0, 100);
		}

		ImGui::Spacing();
		ImGui::Separator();
		ImGui::Spacing();

		if (ImGui::Button("Spawn enemy"))
		{
			auto Enemy = World.entity();
			Enemy.add<EnemiesTag>();
			Enemy.set<TransformComponent>(TransformComponent(glm::vec2(PositionX, PositionY), glm::vec2(ScaleX, ScaleY), glm::degrees(Rotation)));
			Enemy.set<RigidBodyComponent>(RigidBodyComponent(glm::vec2(VelocityX, VelocityY)));
			Enemy.set<SpriteComponent>(SpriteComponent(Sprites[SelectedSpriteIndex], 32, 32, 1));
			Enemy.set<BoxColliderComponent>(BoxColliderComponent(25, 20, glm::vec2(5, 5)));

			const double ProjectileVelocityX = ProjectileSpeed * std::cos(ProjectileAngle);
			const double ProjectileVelocityY = ProjectileSpeed * std::sin(ProjectileAngle);
			Enemy.set<ProjectileEmitterComponent>(ProjectileEmitterComponent(glm::vec2(ProjectileVelocityX, ProjectileVelocityY), static_cast<uint16_t>(ProjectileRepeat * 1000), static_cast<uint16_t>(ProjectileDuration * 1000), 10, false));
			Enemy.set<HealthComponent>(HealthComponent(static_cast<uint8_t>(Health)));

			PositionX = 0;
			PositionY = 0;
			Rotation = 0.0f;
			ProjectileAngle = 0.0f;
			ScaleX = 1;
			ScaleY = 1;
			ProjectileRepeat = 10;
			ProjectileDuration = 10;
			ProjectileSpeed = 100.0f;
			Health = 100;
		}
	}
	ImGui::End();

	ImGuiWindowFlags WindowFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoNav | ImGuiWindowFlags_AlwaysAutoResize;
	ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always, ImVec2(0, 0));
	ImGui::SetNextWindowBgAlpha(0.9f);
	if (ImGui::Begin("Map coordinates", nullptr, WindowFlags))
	{
		ImGui::Text("Map coordinates: (x=%.1f, y=%.1f)", ImGui::GetIO().MousePos.x + Camera.x, ImGui::GetIO().MousePos.y + Camera.y);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), Context.Renderer);
}

static void RenderDebugSystemTask(flecs::iter& Iter, size_t)
{
	RunRenderDebugSystem(Iter.world());
}

void RegisterRenderSystems(flecs::world& World)
{
	World.system("RenderBeginSystem")
		.kind(World.lookup(RenderBeginPhaseName).id())
		.each(RenderBeginSystemTask);

	World.system("RenderSpriteSystem")
		.kind(World.lookup(RenderWorldPhaseName).id())
		.each(RenderSpriteSystemTask);

	World.system("RenderTextSystem")
		.kind(World.lookup(RenderUiPhaseName).id())
		.each(RenderTextSystemTask);

	World.system("RenderHealthBarSystem")
		.kind(World.lookup(RenderUiPhaseName).id())
		.each(RenderHealthBarSystemTask);

	World.system("RenderDebugSystem")
		.kind(World.lookup(RenderDebugPhaseName).id())
		.each(RenderDebugSystemTask);

	World.system("RenderEndSystem")
		.kind(World.lookup(RenderEndPhaseName).id())
		.each(RenderEndSystemTask);
}