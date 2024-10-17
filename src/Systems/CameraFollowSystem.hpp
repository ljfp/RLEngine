#pragma once

#include <flecs.h>
#include "../Components/TransformComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "glm/glm.hpp"
#include <SDL2/SDL.h>
#include <spdlog/spdlog.h>

class CameraFollowSystem
{
public:
	CameraFollowSystem(flecs::world& ecs)
	{
		ecs.system<CameraFollowComponent, TransformComponent>()
			.each([](flecs::entity e, CameraFollowComponent& cameraFollow, TransformComponent& transform) {
				// Camera follow logic here
				SDL_Rect& Camera = *e.get_mut<SDL_Rect>();
				Camera.x = glm::clamp(
					static_cast<int>(transform.Position.x - Camera.w / 2),
					0,
					Game::MapWidth - Camera.w
				);
				Camera.y = glm::clamp(
					static_cast<int>(transform.Position.y - Camera.h / 2),
					0,
					Game::MapHeight - Camera.h
				);

				//spdlog::info("Camera position: ({}, {})", Camera.x, Camera.y);
			});
	}
};
