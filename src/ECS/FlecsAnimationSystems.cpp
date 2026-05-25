#include "FlecsSystems.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/SpriteComponent.hpp"

#include <SDL3/SDL.h>

static void AnimationSystemTask(flecs::iter&, size_t, AnimationComponent& Animation, SpriteComponent& Sprite)
{
	Animation.CurrentFrame = ((SDL_GetTicks() - Animation.StartTime) * Animation.FramesPerSecond / 1000) % Animation.TotalFrames;
	Sprite.SrcRect.x = Sprite.SrcRect.w * Animation.CurrentFrame;
}

void RegisterAnimationSystems(flecs::world& World)
{
	const auto Phase = World.lookup(AnimationPhaseName);
	World.system<AnimationComponent, SpriteComponent>("AnimationSystem")
		.kind(Phase.id())
		.each(AnimationSystemTask);
}