#pragma once

#include <flecs.h>
#include "../Components/AnimationComponent.hpp"
#include "../Components/SpriteComponent.hpp"

class AnimationSystem
{
public:
    AnimationSystem(flecs::world& ecs)
    {
        ecs.system<AnimationComponent, SpriteComponent>()
            .each([](flecs::entity e, AnimationComponent& animation, SpriteComponent& sprite) {
                animation.CurrentFrame = ((SDL_GetTicks() - animation.StartTime) * animation.FramesPerSecond / 1000) % animation.TotalFrames;
                sprite.SrcRect.x = sprite.SrcRect.w * animation.CurrentFrame;
            });
    }
};
