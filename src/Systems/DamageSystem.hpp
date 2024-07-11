#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/CollisionEvent.hpp"
#include <spdlog/spdlog.h>

class DamageSystem : public System
{
public:
	DamageSystem()
	{
		RequireComponent<BoxColliderComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& EventBus)
	{
		EventBus->SubscribeToEvent<CollisionEvent>(this, &DamageSystem::OnCollision);
	}

	void OnCollision(CollisionEvent& Event)
	{
		Entity EntityA = Event.A;
		Entity EntityB = Event.B;
		spdlog::info("The damage system received an event collision between entities {} and {}", EntityA.GetID(), EntityB.GetID());

		if (EntityA.BelongsToGroup("Projectiles") && EntityB.HasTag("Player"))
		{
			OnProjectileHitsPlayer(EntityA, EntityB);
		}

		if (EntityA.HasTag("Player") && EntityB.BelongsToGroup("Projectiles"))
		{
			OnProjectileHitsPlayer(EntityB, EntityA);
		}

		if (EntityA.BelongsToGroup("Projectiles") && EntityB.BelongsToGroup("Enemies"))
		{
			OnProjectileHitsEnemy(EntityA, EntityB);
		}

		if (EntityA.BelongsToGroup("Enemies") && EntityB.BelongsToGroup("Projectiles"))
		{
			OnProjectileHitsEnemy(EntityB, EntityA);
		}
	}

	void Update()
	{
		// TODO: ...
	}

	void OnProjectileHitsPlayer(Entity& AProjectile, Entity& Player)
	{
		const auto AProjectileComponent = AProjectile.GetComponent<ProjectileComponent>();

		if (!AProjectileComponent.IsFriendly)
		{
			auto& PlayerHealth = Player.GetComponent<HealthComponent>();
			PlayerHealth.HealthPercentage -= AProjectileComponent.HitPercentDamage;

			if (PlayerHealth.HealthPercentage <= 0)
			{
				Player.Kill();
			}

			AProjectile.Kill();
		}
	}

	void OnProjectileHitsEnemy(Entity& AProjectile, Entity& Enemy)
	{
		const auto AProjectileComponent = AProjectile.GetComponent<ProjectileComponent>();

		// In this case, the projectile is friendly (it comes from the player), so it should hit the enemy
		if (AProjectileComponent.IsFriendly)
		{
			auto& EnemyHealth = Enemy.GetComponent<HealthComponent>();
			EnemyHealth.HealthPercentage -= AProjectileComponent.HitPercentDamage;

			if (EnemyHealth.HealthPercentage <= 0)
			{
				Enemy.Kill();
			}

			AProjectile.Kill();
		}
	}
};