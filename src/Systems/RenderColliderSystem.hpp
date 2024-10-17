#pragma once

#include <flecs.h>
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL2/SDL.h>

class RenderColliderSystem
{
public:
    RenderColliderSystem(flecs::world& ecs)
    {
        ecs.system<BoxColliderComponent, TransformComponent>()
            .each([](flecs::entity e, BoxColliderComponent& collider, TransformComponent& transform) {
                SDL_Rect ColliderRect = {
                    static_cast<int>(transform.Position.x + collider.Offset.x),
                    static_cast<int>(transform.Position.y + collider.Offset.y),
                    static_cast<int>(collider.Width * transform.Scale.x),
                    static_cast<int>(collider.Height * transform.Scale.y)
                };

                SDL_SetRenderDrawColor(e.world().get<SDL_Renderer>(), 255, 0, 0, 255);
                SDL_RenderDrawRect(e.world().get<SDL_Renderer>(), &ColliderRect);
            });
    }
};
