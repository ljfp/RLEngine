#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../ECS/ECS.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/SpriteComponent.hpp"

#include <SDL3_image/SDL_image.h>
#include <cmath>

class RenderSystem : public System
{
public:
	RenderSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update(SDL_Renderer* Renderer, std::unique_ptr<AssetManager>& AssetManager, SDL_FRect& Camera)
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

			bool IsEntityOutsideCameraView =
			(
				ARenderableEntity.Transform.Position.x + (ARenderableEntity.Transform.Scale.x * ARenderableEntity.Sprite.Width) < Camera.x ||
				ARenderableEntity.Transform.Position.x > Camera.x + Camera.w ||
				ARenderableEntity.Transform.Position.y + (ARenderableEntity.Transform.Scale.y * ARenderableEntity.Sprite.Height) < Camera.y ||
				ARenderableEntity.Transform.Position.y > Camera.y + Camera.h
			);

			// Bypass rendering entities if they are outside the camera view.
			if (IsEntityOutsideCameraView && !ARenderableEntity.Sprite.IsFixed)
			{
				continue;
			}

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

			// Set the source and destination rectangle of our sprite.
			// Round positions to integers to prevent sub-pixel gaps between adjacent tiles.
			SDL_FRect SourceRectangle = Sprite.SrcRect;
			SDL_FRect DestinationRectangle =
			{
				std::round(static_cast<float>(Transform.Position.x - (Sprite.IsFixed ? 0 : Camera.x))),
				std::round(static_cast<float>(Transform.Position.y - (Sprite.IsFixed ? 0 : Camera.y))),
				std::ceil(static_cast<float>(Sprite.Width * Transform.Scale.x)),
				std::ceil(static_cast<float>(Sprite.Height * Transform.Scale.y))
			};

			SDL_RenderTextureRotated
			(
				Renderer,
				AssetManager->GetTexture(Sprite.AssetID),
				&SourceRectangle,
				&DestinationRectangle,
				Transform.Rotation,
				NULL,
				Sprite.Flip
			);
		}
	}
};