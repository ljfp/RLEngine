#pragma once

#include <flecs.h>
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../Events/CollisionEvent.hpp"
#include <spdlog/spdlog.h>

class DamageSystem
{
public:
	DamageSystem(flecs::world& ecs)
	{
		ecs.system<BoxColliderComponent>()
			.each([this](flecs::entity e, BoxColliderComponent& collider) {
				CheckCollisions(e, collider);
			});
	}

	void CheckCollisions(flecs::entity e, BoxColliderComponent& collider)
	{
		auto entities = e.world().filter<BoxColliderComponent>();

		for (auto other : entities)
		{
			if (e == other) continue;

			auto& otherCollider = other.get<BoxColliderComponent>();

			bool isColliding = CheckAABBCollision(
				collider.x, collider.y, collider.width, collider.height,
				otherCollider.x, otherCollider.y, otherCollider.width, otherCollider.height
			);

			if (isColliding)
			{
				spdlog::info("Collision detected between entities {} and {}", e.id(), other.id());
				e.world().event<CollisionEvent>().id(e).id(other).emit();
			}
		}
	}

	bool CheckAABBCollision(double AX, double AY, double AW, double AH, double BX, double BY, double BW, double BH)
	{
		return (AX < BX + BW && AX + AW > BX && AY < BY + BH && AY + AH > BY);
	}

	void OnCollision(flecs::entity e, flecs::entity other)
	{
		spdlog::info("The damage system received an event collision between entities {} and {}", e.id(), other.id());

		if (e.has<ProjectileComponent>() && other.has<HealthComponent>())
		{
			OnProjectileHitsEntity(e, other);
		}

		if (other.has<ProjectileComponent>() && e.has<HealthComponent>())
		{
			OnProjectileHitsEntity(other, e);
		}
	}

	void OnProjectileHitsEntity(flecs::entity projectile, flecs::entity entity)
	{
		const auto& projectileComponent = projectile.get<ProjectileComponent>();

		if (projectileComponent.IsFriendly)
		{
			auto& entityHealth = entity.get_mut<HealthComponent>();
			entityHealth.HealthPercentage -= projectileComponent.HitPercentDamage;

			if (entityHealth.HealthPercentage <= 0)
			{
				entity.destruct();
			}

			projectile.destruct();
		}
	}
};
