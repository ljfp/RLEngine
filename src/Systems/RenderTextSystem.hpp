#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../ECS/ECS.hpp"
#include "../Components/TextLabelComponent.hpp"
#include <SDL2/SDL.h>

class RenderTextSystem : public System
{
public:
	RenderTextSystem() { RequireComponent<TextLabelComponent>(); }

	void Update(SDL_Renderer* Renderer, std::unique_ptr<AssetManager>& AssetManager, const SDL_Rect& Camera)
	{
		for (auto AnEntity : GetSystemEntities())
		{
			const auto ATextLabel = AnEntity.GetComponent<TextLabelComponent>();

			SDL_Surface* TextSurface = TTF_RenderText_Blended
			(
				AssetManager->GetFont(ATextLabel.AssetID),
				ATextLabel.Text.c_str(),
				ATextLabel.Color
			);

			SDL_Texture* TextTexture = SDL_CreateTextureFromSurface(Renderer, TextSurface);
			SDL_FreeSurface(TextSurface);

			int LabelWidth = 0, LabelHeight = 0;
			SDL_QueryTexture(TextTexture, nullptr, nullptr, &LabelWidth, &LabelHeight);

			SDL_Rect DestinationRectangle =
			{
				static_cast<int>(ATextLabel.Position.x - (ATextLabel.IsFixed ? 0 : Camera.x)),
				static_cast<int>(ATextLabel.Position.y - (ATextLabel.IsFixed ? 0 : Camera.y)),
				LabelWidth,
				LabelHeight
			};

			SDL_RenderCopy(Renderer, TextTexture, nullptr, &DestinationRectangle);

			SDL_DestroyTexture(TextTexture);
		}
	}
};