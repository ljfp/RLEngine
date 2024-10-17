#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../Components/TextLabelComponent.hpp"
#include <SDL2/SDL.h>
#include <flecs.h>

class RenderTextSystem
{
public:
    RenderTextSystem(flecs::world& ecs)
    {
        ecs.system<TextLabelComponent>()
            .each([this](flecs::entity e, TextLabelComponent& textLabel) {
                UpdateEntity(e, textLabel);
            });
    }

    void Update(SDL_Renderer* Renderer, std::unique_ptr<AssetManager>& AssetManager, const SDL_Rect& Camera)
    {
        auto entities = e.world().filter<TextLabelComponent>();

        for (auto entity : entities)
        {
            const auto& textLabel = entity.get<TextLabelComponent>();

            SDL_Surface* textSurface = TTF_RenderText_Blended(
                AssetManager->GetFont(textLabel.AssetID),
                textLabel.Text.c_str(),
                textLabel.Color
            );

            SDL_Texture* textTexture = SDL_CreateTextureFromSurface(Renderer, textSurface);
            SDL_FreeSurface(textSurface);

            int labelWidth = 0, labelHeight = 0;
            SDL_QueryTexture(textTexture, nullptr, nullptr, &labelWidth, &labelHeight);

            SDL_Rect destinationRectangle = {
                static_cast<int>(textLabel.Position.x - (textLabel.IsFixed ? 0 : Camera.x)),
                static_cast<int>(textLabel.Position.y - (textLabel.IsFixed ? 0 : Camera.y)),
                labelWidth,
                labelHeight
            };

            SDL_RenderCopy(Renderer, textTexture, nullptr, &destinationRectangle);
            SDL_DestroyTexture(textTexture);
        }
    }

private:
    void UpdateEntity(flecs::entity e, TextLabelComponent& textLabel)
    {
        // This function is intentionally left empty as the actual rendering is done in the Update function.
    }
};
