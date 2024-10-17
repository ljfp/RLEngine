#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include <flecs.h>
#include <SDL2/SDL.h>

class RenderHealthBarSystem
{
public:
    RenderHealthBarSystem(flecs::world& ecs)
    {
        ecs.system<HealthComponent, SpriteComponent, TransformComponent>()
            .each([this](flecs::entity e, HealthComponent& health, SpriteComponent& sprite, TransformComponent& transform) {
                UpdateEntity(e, health, sprite, transform);
            });
    }

    void UpdateEntity(flecs::entity e, HealthComponent& health, SpriteComponent& sprite, TransformComponent& transform)
    {
        SDL_Renderer* Renderer = e.world().get<SDL_Renderer>();
        const SDL_Rect& Camera = e.world().get<SDL_Rect>();

        SDL_Color HealthBarColor = { 255, 255, 255 };

        if (health.HealthPercentage >= 0 && health.HealthPercentage < 40)
        {
            // 0-39 = red
            HealthBarColor = { 255, 0, 0 };
        }
        else if (health.HealthPercentage >= 40 && health.HealthPercentage < 80)
        {
            // 40-79 = yellow
            HealthBarColor = { 255, 255, 0 };
        }
        else if (health.HealthPercentage >= 80 && health.HealthPercentage <= 100)
        {
            // 80-100 = green
            HealthBarColor = { 0, 255, 0 };
        }

        // Position of the health bar indicator in the middle-bottom part of the sprite
        int HealthBarWidth = 15;
        int HealthBarHeight = 3;
        double HealthBarPositionX = (transform.Position.x + (sprite.Width * transform.Scale.x)) - Camera.x;
        double HealthBarPositionY = (transform.Position.y) - Camera.y;

        SDL_Rect HealthBarRect =
        {
            static_cast<int>(HealthBarPositionX),
            static_cast<int>(HealthBarPositionY),
            static_cast<int>(HealthBarWidth * (health.HealthPercentage / 100.0)),
            static_cast<int>(HealthBarHeight)
        };
        SDL_SetRenderDrawColor(Renderer, HealthBarColor.r, HealthBarColor.g, HealthBarColor.b, 255);
        SDL_RenderFillRect(Renderer, &HealthBarRect);

        // Render the health percentage label indicator
        std::string HealthText = std::to_string(health.HealthPercentage) + "%";
        SDL_Surface* TextSurface = TTF_RenderText_Blended(e.world().get<AssetManager>()->GetFont("pico8-font-5"), HealthText.c_str(), HealthBarColor);
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
};
