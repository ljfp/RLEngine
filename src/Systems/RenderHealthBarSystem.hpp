#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../ECS/ECS.hpp"

#include <SDL2/SDL.h>

class RenderHealthBarSystem : public System
{
public:
	RenderHealthBarSystem()
	{
		RequireComponent<HealthComponent>();
		RequireComponent<SpriteComponent>();
		RequireComponent<TransformComponent>();
	}

	void Update(SDL_Renderer* Renderer, const std::unique_ptr<AssetManager>& AssetManager, const SDL_Rect& Camera)
	{
		for (auto AnEntity : GetSystemEntities())
		{
			const auto Transform = AnEntity.GetComponent<TransformComponent>();
			const auto Health = AnEntity.GetComponent<HealthComponent>();
			const auto Sprite = AnEntity.GetComponent<SpriteComponent>();

			SDL_Color HealthBarColor = { 255, 255, 255 };

			if (Health.HealthPercentage >= 0 && Health.HealthPercentage < 40)
			{
				// 0-39 = red
				HealthBarColor = { 255, 0, 0 };
			}
			else if (Health.HealthPercentage >= 40 && Health.HealthPercentage < 80)
			{
				// 40-79 = yellow
				HealthBarColor = { 255, 255, 0 };
			}
			else if (Health.HealthPercentage >= 80 && Health.HealthPercentage <= 100)
			{
				// 80-100 = green
				HealthBarColor = { 0, 255, 0 };
			}

			// Position of the health bar indicator in the middle-bottom part of the sprite
			int HealthBarWidth = 15;
			int HealthBarHeight = 3;
			double HealthBarPositionX = (Transform.Position.x + (Sprite.Width * Transform.Scale.x)) - Camera.x;
			double HealthBarPositionY = (Transform.Position.y) - Camera.y;

			SDL_Rect HealthBarRect =
			{
				static_cast<int>(HealthBarPositionX),
				static_cast<int>(HealthBarPositionY),
				static_cast<int>(HealthBarWidth * (Health.HealthPercentage / 100.0)),
				static_cast<int>(HealthBarHeight)
			};
			SDL_SetRenderDrawColor(Renderer, HealthBarColor.r, HealthBarColor.g, HealthBarColor.b, 255);
			SDL_RenderFillRect(Renderer, &HealthBarRect);

			// Render the health percentage label indicator
			std::string HealthText = std::to_string(Health.HealthPercentage) + "%";
			SDL_Surface* TextSurface = TTF_RenderText_Blended(AssetManager->GetFont("pico8-font-5"), HealthText.c_str(), HealthBarColor);
			SDL_Texture* TextTexture = SDL_CreateTextureFromSurface(Renderer, TextSurface);
			SDL_FreeSurface(TextSurface);

			int LabelWidth = 0, LabelHeight = 0;
			SDL_QueryTexture(TextTexture, nullptr, nullptr, &LabelWidth, &LabelHeight);
			SDL_Rect HealthBarTextRectangle =
			{
				static_cast<int>(HealthBarPositionX),
				static_cast<int>(HealthBarPositionY) + 5,
				LabelWidth,
				LabelHeight
			};

			SDL_RenderCopy(Renderer, TextTexture, nullptr, &HealthBarTextRectangle);
			SDL_DestroyTexture(TextTexture);
		}
	}
};