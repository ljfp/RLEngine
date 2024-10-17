#pragma once

#include <flecs.h>
#include "../Components/ProjectileComponent.hpp"
#include <SDL2/SDL.h>

class ProjectileLifecycleSystem
{
public:
    ProjectileLifecycleSystem(flecs::world& ecs)
    {
        ecs.system<ProjectileComponent>()
            .each([](flecs::entity e, ProjectileComponent& projectile) {
                if (SDL_GetTicks() - projectile.StartTime > projectile.Duration)
                {
                    e.destruct();
                }
            });
    }
};
