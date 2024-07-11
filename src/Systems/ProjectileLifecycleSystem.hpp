#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/ProjectileComponent.hpp"
#include <SDL2/SDL.h>

class ProjectileLifecycleSystem : public System
{
public:
	ProjectileLifecycleSystem()
	{
		RequireComponent<ProjectileComponent>();
	}

	void Update()
	{
		for (auto Entity : GetSystemEntities())
		{
			auto& AProjectile = Entity.GetComponent<ProjectileComponent>();
			if (SDL_GetTicks() - AProjectile.StartTime > AProjectile.Duration)
			{
				Entity.Kill();
			}
		}
	}
};