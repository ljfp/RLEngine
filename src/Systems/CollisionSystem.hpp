#pragma once

#include <flecs.h>
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../EventBus/Event.hpp"
#include "../Events/CollisionEvent.hpp"

#include <spdlog/spdlog.h>

class CollisionSystem
{
public:
	CollisionSystem(flecs::world& ecs)
	{
		ecs.system<BoxColliderComponent, TransformComponent>()
			.each([this](flecs::entity e, BoxColliderComponent& collider, TransformComponent& transform) {
				CheckCollisions(e, collider, transform);
			});
	}

	void CheckCollisions(flecs::entity e, BoxColliderComponent& collider, TransformComponent& transform)
	{
		auto entities = e.world().filter<BoxColliderComponent, TransformComponent>();

		for (auto other : entities)
		{
			if (e == other) continue;

			auto& otherTransform = other.get<TransformComponent>();
			auto& otherCollider = other.get<BoxColliderComponent>();

			bool isColliding = CheckAABBCollision(
				transform.Position.x + collider.Offset.x, transform.Position.y + collider.Offset.y, collider.Width * transform.Scale.x, collider.Height * transform.Scale.y,
				otherTransform.Position.x + otherCollider.Offset.x, otherTransform.Position.y + otherCollider.Offset.y, otherCollider.Width * otherTransform.Scale.x, otherCollider.Height * otherTransform.Scale.y
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
};
