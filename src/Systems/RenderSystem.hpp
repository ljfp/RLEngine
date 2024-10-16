#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../ECS/ECS.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/SpriteComponent.hpp"

#include <SDL2/SDL_image.h>

class RenderSystem : public System
{
public:
	RenderSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(SDL_Renderer* Renderer, std::unique_ptr<AssetManager>& AssetManager, SDL_Rect& Camera)
	{
		// Create a vector with both Sprite and Transform components of all entities.
		struct RenderableEntity
		{
			TransformComponent Transform;
			SpriteComponent Sprite;
		};
		std::vector<RenderableEntity> RenderableEntities;
		for (auto AnEntity : GetSystemEntities())
		{
			RenderableEntity ARenderableEntity;
			ARenderableEntity.Transform = AnEntity.GetComponent<TransformComponent>();
			ARenderableEntity.Sprite = AnEntity.GetComponent<SpriteComponent>();
			RenderableEntities.emplace_back(ARenderableEntity);
		}

		// Sort the vector by the ZIndex value of the Sprite component.
		std::sort
		(
			RenderableEntities.begin(),
			RenderableEntities.end(),
			[](const RenderableEntity& A, const RenderableEntity& B)
			{
				return A.Sprite.ZIndex < B.Sprite.ZIndex;
			}
		);

		for (auto AnEntity : RenderableEntities)
		{
			const auto Transform = AnEntity.Transform;
			const auto Sprite = AnEntity.Sprite;

			// Set the source and destination rectangle of our sprite
			SDL_Rect SourceRectangle = Sprite.SrcRect;
			SDL_Rect DestinationRectangle =
			{
				static_cast<int>(Transform.Position.x - (Sprite.IsFixed ? 0 : Camera.x)),
				static_cast<int>(Transform.Position.y - (Sprite.IsFixed ? 0 : Camera.y)),
				static_cast<int>(Sprite.Width * Transform.Scale.x),
				static_cast<int>(Sprite.Height * Transform.Scale.y)
			};

			SDL_RenderCopyEx
			(
				Renderer,
				AssetManager->GetTexture(Sprite.AssetID),
				&SourceRectangle,
				&DestinationRectangle,
				Transform.Rotation,
				NULL,
				SDL_FLIP_NONE
			);
		}
	}
};