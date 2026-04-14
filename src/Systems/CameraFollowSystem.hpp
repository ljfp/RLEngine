#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "glm/glm.hpp"
#include <SDL3/SDL.h>
#include <spdlog/spdlog.h>
#include <algorithm>

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

			// Center the camera on the entity
			float DesiredX = static_cast<float>(ATransform.Position.x) - Camera.w / 2.0f;
			float DesiredY = static_cast<float>(ATransform.Position.y) - Camera.h / 2.0f;

			float MaxX = static_cast<float>(Game::MapWidth) - Camera.w;
			float MaxY = static_cast<float>(Game::MapHeight) - Camera.h;

			// When the map is smaller than the screen, center the map in the viewport
			if (MaxX <= 0.0f)
			{
				Camera.x = MaxX / 2.0f;
			}
			else
			{
				Camera.x = glm::clamp(DesiredX, 0.0f, MaxX);
			}

			if (MaxY <= 0.0f)
			{
				Camera.y = MaxY / 2.0f;
			}
			else
			{
				Camera.y = glm::clamp(DesiredY, 0.0f, MaxY);
			}
		}
	}
};