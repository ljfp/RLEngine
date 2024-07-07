#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"


class MovementSystem : public System
{
public:
	MovementSystem()
	{
		RequireComponent<TransformComponent>();
		RequireComponent<RigidBodyComponent>();
	}

	void Update(double DeltaTime)
	{
		for (auto entity : GetSystemEntities())
		{
			auto& transform = entity.GetComponent<TransformComponent>();
			const auto& rigidBody = entity.GetComponent<RigidBodyComponent>();

			transform.Position.x += rigidBody.Velocity.x * DeltaTime;
			transform.Position.y += rigidBody.Velocity.y * DeltaTime;

			bool IsEntityOutsideOfBounds =
			(
				transform.Position.x < 0 ||
				transform.Position.x > Game::MapWidth ||
				transform.Position.y < 0 ||
				transform.Position.y > Game::MapHeight
			);

			// Kill all entites that move outside of the map boundaries.
			if (IsEntityOutsideOfBounds && !entity.HasTag("Player"))
			{
				entity.Kill();
			}
		}
	}
};