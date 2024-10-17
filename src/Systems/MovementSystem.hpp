#pragma once

#include <flecs.h>
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/CollisionEvent.hpp"

class MovementSystem
{
public:
	MovementSystem(flecs::world& ecs)
	{
		ecs.system<TransformComponent, RigidBodyComponent>()
			.each([this](flecs::entity e, TransformComponent& transform, RigidBodyComponent& rigidBody) {
				UpdateEntity(e, transform, rigidBody);
			});

		ecs.observer<CollisionEvent>()
			.event<CollisionEvent>()
			.each([this](flecs::entity e, CollisionEvent& event) {
				OnCollision(event);
			});
	}

	void UpdateEntity(flecs::entity e, TransformComponent& transform, RigidBodyComponent& rigidBody)
	{
		double DeltaTime = e.world().delta_time();

		transform.Position.x += rigidBody.Velocity.x * DeltaTime;
		transform.Position.y += rigidBody.Velocity.y * DeltaTime;

		// Prevent the player from moving outside the map.
		if (e.has<TagComponent<Player>>())
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

		// Kill all entities that move outside of the map boundaries.
		if (IsEntityOutsideOfBounds && !e.has<TagComponent<Player>>())
		{
			e.destruct();
		}
	}

	void OnCollision(CollisionEvent& Event)
	{
		flecs::entity EntityA = Event.A;
		flecs::entity EntityB = Event.B;
		spdlog::info("The damage system received an event collision between entities {} and {}", EntityA.id(), EntityB.id());

		if (EntityA.has<TagComponent<Enemy>>() && EntityB.has<TagComponent<Obstacle>>())
		{
			OnEnemyHitsObstacle(EntityA, EntityB);
		}

		if (EntityA.has<TagComponent<Obstacle>>() && EntityB.has<TagComponent<Enemy>>())
		{
			OnEnemyHitsObstacle(EntityB, EntityA);
		}
	}

	void OnEnemyHitsObstacle(flecs::entity Enemy, flecs::entity Obstacle)
	{
		if (Enemy.has<RigidBodyComponent>() && Enemy.has<SpriteComponent>())
		{
			auto& RigidBody = Enemy.get_mut<RigidBodyComponent>();
			auto& Sprite = Enemy.get_mut<SpriteComponent>();

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
