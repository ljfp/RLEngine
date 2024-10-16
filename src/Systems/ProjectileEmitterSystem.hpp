#pragma once

#include "../ECS/ECS.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/KeyPressedEvent.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <spdlog/spdlog.h>


class ProjectileEmitterSystem : public System
{
public:
	ProjectileEmitterSystem()
	{
		RequireComponent<ProjectileEmitterComponent>();
		RequireComponent<TransformComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& EventBus)
	{
		EventBus->SubscribeToEvent<KeyPressedEvent>(this, &ProjectileEmitterSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& Event)
	{
		if (Event.KeyCode == SDLK_SPACE)
		{
			spdlog::info("Space key pressed");
			for (auto AnEntity : GetSystemEntities())
			{
				if (AnEntity.HasComponent<CameraFollowComponent>())
				{
					const auto AProjectileEmitter = AnEntity.GetComponent<ProjectileEmitterComponent>();
					const auto ATransform = AnEntity.GetComponent<TransformComponent>();
					const auto ARigidBody = AnEntity.GetComponent<RigidBodyComponent>();

					// Projectile position
					glm::vec2 ProjectilePosition = ATransform.Position;
					if (AnEntity.HasComponent<SpriteComponent>())
					{
						const auto ASprite = AnEntity.GetComponent<SpriteComponent>();
						ProjectilePosition.x += (ATransform.Scale.x * ASprite.Width / 2);
						ProjectilePosition.y += (ATransform.Scale.y * ASprite.Height / 2);
					}

					// If the parent entity direction is controlled by the keyboard modify the direction of the projectile accordingly.
					glm::vec2 ProjectileVelocity = AProjectileEmitter.ProjectileVelocity;
					int16_t DirectionX = 0;
					int16_t DirectionY = 0;
					if (ARigidBody.Velocity.x > 0) DirectionX = +1;
					if (ARigidBody.Velocity.x < 0) DirectionX = -1;
					if (ARigidBody.Velocity.y > 0) DirectionY = +1;
					if (ARigidBody.Velocity.y < 0) DirectionY = -1;
					ProjectileVelocity.x = DirectionX * AProjectileEmitter.ProjectileVelocity.x;
					ProjectileVelocity.y = DirectionY * AProjectileEmitter.ProjectileVelocity.y;

					// Emit a projectile
					Entity AProjectile = AnEntity.registry->CreateEntity();
					AProjectile.Group("Projectiles");
					AProjectile.AddComponent<TransformComponent>(ProjectilePosition, glm::vec2(1.0, 1.0), 0.0);
					AProjectile.AddComponent<RigidBodyComponent>(ProjectileVelocity);
					AProjectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
					AProjectile.AddComponent<BoxColliderComponent>(4, 4);
					AProjectile.AddComponent<ProjectileComponent>(AProjectileEmitter.IsFriendly, AProjectileEmitter.HitPercentDamage, AProjectileEmitter.ProjectileDuration);
				}
			}
		}
	}

	void Update(std::unique_ptr<Registry>& Registry)
	{
		for (auto AnEntity : GetSystemEntities())
		{
			auto& AProjectileEmitter = AnEntity.GetComponent<ProjectileEmitterComponent>();
			const auto ATransform = AnEntity.GetComponent<TransformComponent>();

			// If emission frequency is zero, bypass re-emission logic
			if (AProjectileEmitter.ProjectileFrequency == 0) continue;

			// Check if its tome to re-emit a new projectile
			if (SDL_GetTicks() - AProjectileEmitter.LastEmissionTime > AProjectileEmitter.ProjectileFrequency)
			{
				// Projectile position
				glm::vec2 ProjectilePosition = ATransform.Position;
				if (AnEntity.HasComponent<SpriteComponent>())
				{
					const auto ASprite = AnEntity.GetComponent<SpriteComponent>();
					ProjectilePosition.x += (ATransform.Scale.x * ASprite.Width / 2);
					ProjectilePosition.y += (ATransform.Scale.y * ASprite.Height / 2);
				}
				// Emit a projectile
				Entity AProjectile = Registry->CreateEntity();
				AProjectile.Group("Projectiles");
				AProjectile.AddComponent<TransformComponent>(ProjectilePosition, glm::vec2(1.0, 1.0), 0.0);
				AProjectile.AddComponent<RigidBodyComponent>(AProjectileEmitter.ProjectileVelocity);
				AProjectile.AddComponent<SpriteComponent>("bullet-image", 4, 4, 4);
				AProjectile.AddComponent<BoxColliderComponent>(4, 4);
				AProjectile.AddComponent<ProjectileComponent>(AProjectileEmitter.IsFriendly, AProjectileEmitter.HitPercentDamage, AProjectileEmitter.ProjectileDuration);

				// Update LastEmissionTime
				AProjectileEmitter.LastEmissionTime = SDL_GetTicks();
			}
		}
	}
};