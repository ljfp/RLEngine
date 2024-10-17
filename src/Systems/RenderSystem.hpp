#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/SpriteComponent.hpp"

#include <SDL2/SDL_image.h>
#include <flecs.h>

class RenderSystem
{
public:
    RenderSystem(flecs::world& ecs)
    {
        ecs.system<TransformComponent, SpriteComponent>()
            .each([this](flecs::entity e, TransformComponent& transform, SpriteComponent& sprite) {
                UpdateEntity(e, transform, sprite);
            });
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
        auto entities = e.world().filter<TransformComponent, SpriteComponent>();
        for (auto entity : entities)
        {
            RenderableEntity ARenderableEntity;
            ARenderableEntity.Transform = entity.get<TransformComponent>();
            ARenderableEntity.Sprite = entity.get<SpriteComponent>();

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
                Sprite.Flip
            );
        }
    }

private:
    void UpdateEntity(flecs::entity e, TransformComponent& transform, SpriteComponent& sprite)
    {
        // This function is intentionally left empty as the actual rendering is done in the Update function.
    }
};
