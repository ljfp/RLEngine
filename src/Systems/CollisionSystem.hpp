#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../EventBus/Event.hpp"
#include "../Events/CollisionEvent.hpp"

#include <spdlog/spdlog.h>

class CollisionSystem : public System
{
public:
	CollisionSystem()
	{
		RequireComponent<BoxColliderComponent>();
		RequireComponent<TransformComponent>();
	}

	void Update(std::unique_ptr<EventBus>& EventBus)
	{
		// TODO: check all entities that have a BoxColliderComponent and a TransformComponent to see if they are colliding with each other.
		auto Entities = GetSystemEntities();

		for (auto i = Entities.begin(); i != Entities.end(); i++)
		{
			Entity A = *i;

			auto ATransform = A.GetComponent<TransformComponent>();
			auto ACollider = A.GetComponent<BoxColliderComponent>();

			for (auto j = i; j != Entities.end(); j++)
			{
				Entity B = *j;

				// Skip if A and B are the same entity
				if (A == B) { continue; }

				auto BTransform = B.GetComponent<TransformComponent>();
				auto BCollider = B.GetComponent<BoxColliderComponent>();

				// Check for collision using AABB (Axis-Aligned Bounding Box)
				bool IsColliding = CheckAABBCollision(
					ATransform.Position.x + ACollider.Offset.x, ATransform.Position.y + ACollider.Offset.y, ACollider.Width * ATransform.Scale.x, ACollider.Height * ATransform.Scale.y,
					BTransform.Position.x + BCollider.Offset.x, BTransform.Position.y + BCollider.Offset.y, BCollider.Width * BTransform.Scale.x, BCollider.Height * BTransform.Scale.y
				);
				if (IsColliding)
				{
					spdlog::info("Collision detected between entities {} and {}", A.GetID(), B.GetID());

					EventBus->EmitEvent<CollisionEvent>(A, B);
				}
			}
		}
	}

	bool CheckAABBCollision(double AX, double AY, double AW, double AH, double BX, double BY, double BW, double BH)
	{
		return (AX < BX + BW && AX + AW > BX && AY < BY + BH && AY + AH > BY);
	}

};