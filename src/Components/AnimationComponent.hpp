#pragma once

#include <SDL2/SDL.h>

struct AnimationComponent
{
	uint8_t TotalFrames;
	uint8_t CurrentFrame;
	uint8_t FramesPerSecond;
	bool Loop;
	uint32_t StartTime;


	AnimationComponent(uint8_t TotalFrames = 1, uint8_t FramesPerSecond = 1, bool Loop = true)
	{
		this->TotalFrames = TotalFrames;
		this->CurrentFrame = 1;
		this->FramesPerSecond = FramesPerSecond;
		this->Loop = Loop;
		this->StartTime = SDL_GetTicks();
	}
};