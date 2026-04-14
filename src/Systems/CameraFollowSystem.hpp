#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "glm/glm.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>

class CameraFollowSystem : public System
{
public:
	CameraFollowSystem()
	{
		RequireComponent<CameraFollowComponent>();
		RequireComponent<TransformComponent>();
	}

	void Update(SDL_FRect& Camera)
	{
		auto Entities = GetSystemEntities();

		for (auto AnEntity : Entities)
		{
			auto ATransform = AnEntity.GetComponent<TransformComponent>();

			// Camera follow logic here
			Camera.x = glm::clamp(
				static_cast<float>(ATransform.Position.x - Camera.w / 2),
				0.0f,
				static_cast<float>(Game::MapWidth) - Camera.w
			);
			Camera.y = glm::clamp(
				static_cast<float>(ATransform.Position.y - Camera.h / 2),
				0.0f,
				static_cast<float>(Game::MapHeight) - Camera.h
			);

			//spdlog::info("Camera position: ({}, {})", Camera.x, Camera.y);
		}
	}
};