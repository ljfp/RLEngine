#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../ECS/ECS.hpp"
#include "../Components/TextLabelComponent.hpp"
#include <SDL3/SDL.h>

class RenderTextSystem : public System
{
public:
	RenderTextSystem() { RequireComponent<TextLabelComponent>(); }

	void Update(SDL_Renderer* Renderer, std::unique_ptr<AssetManager>& AssetManager, const SDL_FRect& Camera)
	{
		for (auto AnEntity : GetSystemEntities())
		{
			const auto ATextLabel = AnEntity.GetComponent<TextLabelComponent>();

			SDL_Surface* TextSurface = TTF_RenderText_Blended
			(
				AssetManager->GetFont(ATextLabel.AssetID),
				ATextLabel.Text.c_str(),
				0,
				ATextLabel.Color
			);

			SDL_Texture* TextTexture = SDL_CreateTextureFromSurface(Renderer, TextSurface);
			SDL_DestroySurface(TextSurface);

			float LabelWidth = 0, LabelHeight = 0;
			SDL_GetTextureSize(TextTexture, &LabelWidth, &LabelHeight);

			SDL_FRect DestinationRectangle =
			{
				static_cast<float>(ATextLabel.Position.x - (ATextLabel.IsFixed ? 0 : Camera.x)),
				static_cast<float>(ATextLabel.Position.y - (ATextLabel.IsFixed ? 0 : Camera.y)),
				LabelWidth,
				LabelHeight
			};

			SDL_RenderTexture(Renderer, TextTexture, nullptr, &DestinationRectangle);
			SDL_DestroyTexture(TextTexture);
		}
	}
};