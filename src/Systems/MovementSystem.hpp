#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/CollisionEvent.hpp"

class MovementSystem : public System
{
public:
	MovementSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void SubscribeToEvents(const std::unique_ptr<EventBus>& EventBus)
	{
		EventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystem::OnCollision);
	}

	void Update(double DeltaTime)
	{
		for (auto entity : GetSystemEntities())
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			const auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

			transform.Position.x += rigidBody.Velocity.x * DeltaTime;
			transform.Position.y += rigidBody.Velocity.y * DeltaTime;

			// Prevent the player from moving outside the map.
			if (entity.HasTag("Player"))
			{
				uint8_t PaddingLeft = 10;
				uint8_t PaddingTop = 10;
				uint8_t PaddingRight = 50;
				uint8_t PaddingBottom = 50;
				transform.Position.x = transform.Position.x < PaddingLeft ? PaddingLeft : transform.Position.x;
				transform.Position.x = transform.Position.x > Game::MapWidth - PaddingRight ? Game::MapWidth - PaddingRight : transform.Position.x;
				transform.Position.y = transform.Position.y < PaddingTop ? PaddingTop : transform.Position.y;
				transform.Position.y = transform.Position.y > Game::MapHeight - PaddingBottom ? Game::MapHeight - PaddingBottom : transform.Position.y;
			}

			uint8_t Padding = 100; // In pixels
			bool IsEntityOutsideOfBounds =
			(
				transform.Position.x < -Padding ||
				transform.Position.x > Game::MapWidth + Padding ||
				transform.Position.y < -Padding ||
				transform.Position.y > Game::MapHeight + Padding
			);

			// Kill all entites that move outside of the map boundaries.
			if (IsEntityOutsideOfBounds && !entity.HasTag("Player"))
			{
				entity.Kill();
			}
		}
	}

	void OnCollision(CollisionEvent& Event)
	{
		Entity EntityA = Event.A;
		Entity EntityB = Event.B;
		spdlog::info("The damage system received an event collision between entities {} and {}", EntityA.GetID(), EntityB.GetID());

		if (EntityA.BelongsToGroup("Enemies") && EntityB.BelongsToGroup("Obstacles"))
		{
			OnEnemyHitsObstacle(EntityA, EntityB);
		}

		if (EntityA.BelongsToGroup("Obstacles") && EntityB.BelongsToGroup("Enemies"))
		{
			OnEnemyHitsObstacle(EntityB, EntityA);
		}
	}

	void OnEnemyHitsObstacle(Entity Enemy, Entity Obstacle)
	{
		if (Enemy.HasComponent<RigidBodyComponent>() && Enemy.HasComponent<SpriteComponent>())
		{
			auto& RigidBody = Enemy.GetComponent<RigidBodyComponent>();
			auto& Sprite = Enemy.GetComponent<SpriteComponent>();

			if (RigidBody.Velocity.x != 0)
			{
				RigidBody.Velocity.x *= -1;
				Sprite.Flip = (Sprite.Flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
			}

			if (RigidBody.Velocity.y != 0)
			{
				RigidBody.Velocity.y *= -1;
				Sprite.Flip = (Sprite.Flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
			}

		}
	}
};