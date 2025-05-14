#pragma once

#include "../ECS/FlecsBridge.hpp"
#include "../AssetManager/AssetManager.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/SpriteComponent.hpp"

#include <SDL2/SDL_image.h>
#include <vector>
#include <algorithm>
#include <spdlog/spdlog.h>

class RenderSystemFlecs : public FlecsBridgeSystem
{
public:
    RenderSystemFlecs() = default;
    
    void Update(SDL_Renderer* Renderer, std::unique_ptr<AssetManager>& AssetManager, SDL_Rect& Camera, bool isDebug = false)
    {
        if (!m_world) {
            spdlog::error("RenderSystemFlecs: World pointer is null!");
            return;
        }
        
        // Create a vector with both Sprite and Transform components of all entities.
        struct RenderableEntity
        {
            TransformComponent Transform;
            SpriteComponent Sprite;
            flecs::entity Entity;
        };
        std::vector<RenderableEntity> RenderableEntities;
        
        // Find all entities with SpriteComponent and TransformComponent
        int entityCount = 0;
        m_world->each([&](flecs::entity entity, 
                      SpriteComponent& sprite, 
                      TransformComponent& transform)
        {
            entityCount++;
            bool IsEntityOutsideCameraView =
            (
                transform.Position.x + (transform.Scale.x * sprite.Width) < Camera.x ||
                transform.Position.x > Camera.x + Camera.w ||
                transform.Position.y + (transform.Scale.y * sprite.Height) < Camera.y ||
                transform.Position.y > Camera.y + Camera.h
            );

            // Bypass rendering entities if they are outside the camera view.
            if (IsEntityOutsideCameraView && !sprite.IsFixed)
            {
                return;
            }

            RenderableEntities.push_back({transform, sprite, entity});
        });
        
        spdlog::info("RenderSystemFlecs: Found {} entities with Sprite and Transform components, {} are visible", 
                    entityCount, RenderableEntities.size());

        // Sort the vector by the ZIndex value of the Sprite component.
        std::sort(
            RenderableEntities.begin(),
            RenderableEntities.end(),
            [](const RenderableEntity& A, const RenderableEntity& B)
            {
                return A.Sprite.ZIndex < B.Sprite.ZIndex;
            }
        );

        // Render entities according to Z-index
        for (auto& Entity : RenderableEntities)
        {
            SDL_Texture* Texture = AssetManager->GetTexture(Entity.Sprite.AssetID);
            
            // Destination rectangle
            SDL_Rect DestinationRectangle = {
                static_cast<int>(Entity.Transform.Position.x - (Entity.Sprite.IsFixed ? 0 : Camera.x)),
                static_cast<int>(Entity.Transform.Position.y - (Entity.Sprite.IsFixed ? 0 : Camera.y)),
                static_cast<int>(Entity.Sprite.Width * Entity.Transform.Scale.x),
                static_cast<int>(Entity.Sprite.Height * Entity.Transform.Scale.y)
            };
            
            // Only draw debug rectangles if debug mode is enabled
            if (isDebug) {
                // Draw a yellow rectangle
                SDL_SetRenderDrawColor(Renderer, 255, 255, 0, 255);
                SDL_RenderFillRect(Renderer, &DestinationRectangle);
                SDL_SetRenderDrawColor(Renderer, 255, 0, 0, 255);
                SDL_RenderDrawRect(Renderer, &DestinationRectangle);
            }
            
            // If texture is available, render it as well
            if (Texture) {
                // Source rectangle
                SDL_Rect SourceRectangle = Entity.Sprite.SrcRect;
                
                SDL_RenderCopyEx(
                    Renderer,
                    Texture,
                    &SourceRectangle,
                    &DestinationRectangle,
                    Entity.Transform.Rotation,
                    NULL,
                    Entity.Sprite.Flip
                );
            } else {
                spdlog::warn("Missing texture for asset ID: {}", Entity.Sprite.AssetID);
            }
        }
    }
    
    void Run(flecs::iter& it) override {
        // This is a placeholder - the actual rendering happens in the Update method
        // which is called manually from Game::Render()
    }
    
    void SetupQuery(flecs::world& world, flecs::system& system) override {
        // Store the world reference
        m_world = &world;
        spdlog::info("RenderSystemFlecs: World reference set in SetupQuery");
    }
    
    // Override from FlecsBridgeSystem
    void SetWorld(flecs::world* world) override {
        m_world = world;
        spdlog::info("RenderSystemFlecs: World reference set in SetWorld");
    }
    
private:
    flecs::world* m_world = nullptr;
}; 