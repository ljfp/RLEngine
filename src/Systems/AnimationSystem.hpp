#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/SpriteComponent.hpp"

class AnimationSystem : public System
{
public:
	AnimationSystem()
	{
		RequireComponent<AnimationComponent>();
		RequireComponent<SpriteComponent>();
	}

	void Update()
	{
		for (Entity AnEntity : GetSystemEntities())
		{
			AnimationComponent& Animation = AnEntity.GetComponent<AnimationComponent>();
			SpriteComponent& Sprite = AnEntity.GetComponent<SpriteComponent>();

			Animation.CurrentFrame = ((SDL_GetTicks() - Animation.StartTime) * Animation.FramesPerSecond / 1000) % Animation.TotalFrames;
			Sprite.SrcRect.x = Sprite.SrcRect.w * Animation.CurrentFrame;
		}
	}
};