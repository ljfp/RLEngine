#pragma once

#include <glm/glm.hpp>
#include <SDL2/SDL.h>

struct ProjectileEmitterComponent
{
	glm::vec2 ProjectileVelocity;
	uint16_t ProjectileFrequency;
	uint16_t ProjectileDuration;
	uint8_t HitPercentDamage;
	bool IsFriendly;
	uint16_t LastEmissionTime;

	ProjectileEmitterComponent
	(
		glm::vec2 ProjectileVelocity = glm::vec2(0),
		uint16_t ProjectileFrequency = 0,
		uint16_t ProjectileDuration = 10000,
		uint8_t HitPercentDamage = 10,
		bool IsFriendly = false
	)
	{
		this->ProjectileVelocity = ProjectileVelocity;
		this->ProjectileFrequency = ProjectileFrequency;
		this->ProjectileDuration = ProjectileDuration;
		this->HitPercentDamage = HitPercentDamage;
		this->IsFriendly = IsFriendly;
		this->LastEmissionTime = SDL_GetTicks();
	}
};