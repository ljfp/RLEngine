#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3/SDL.h>

class RenderColliderSystem : public System
{
public:
	RenderColliderSystem()
	{
		RequireComponent<BoxColliderComponent>();
		RequireComponent<TransformComponent>();
	}

	void Update(SDL_Renderer *Renderer, SDL_FRect& Camera)
	{
		auto Entities = GetSystemEntities();

		for (auto AnEntity : Entities)
		{
			const auto ATransform = AnEntity.GetComponent<TransformComponent>();
			const auto ACollider = AnEntity.GetComponent<BoxColliderComponent>();

			SDL_FRect ColliderRect = {
				static_cast<float>(ATransform.Position.x + ACollider.Offset.x - Camera.x),
				static_cast<float>(ATransform.Position.y + ACollider.Offset.y - Camera.y),
				static_cast<float>(ACollider.Width * ATransform.Scale.x),
				static_cast<float>(ACollider.Height * ATransform.Scale.y)
			};

			SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
			SDL_RenderRect(Renderer, &ColliderRect);
		}
	}
};