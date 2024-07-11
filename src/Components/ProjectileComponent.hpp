#pragma once

#include <SDL2/SDL.h>
#include <stdint.h>

struct ProjectileComponent
{
	bool IsFriendly;
	uint8_t HitPercentDamage;
	uint16_t Duration;
	uint16_t StartTime;

	ProjectileComponent(bool IsFriendly = false, uint8_t HitPercentDamage = 0, uint16_t Duration = 0)
	{
		this->IsFriendly = IsFriendly;
		this->HitPercentDamage = HitPercentDamage;
		this->Duration = Duration;
		this->StartTime = SDL_GetTicks();
	}
};