#include "FlecsSystems.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3/SDL.h>

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <vector>

static bool IsAlive(flecs::world& World, flecs::entity Entity)
{
	return Entity.id() != 0 && World.is_alive(Entity.id());
}

static bool CheckAABBCollision(double AX, double AY, double AW, double AH, double BX, double BY, double BW, double BH)
{
	return AX < BX + BW && AX + AW > BX && AY < BY + BH && AY + AH > BY;
}

static void HandleProjectileHitsHealthTarget(flecs::entity ProjectileEntity, flecs::entity TargetEntity, bool TargetIsPlayer)
{
	if (!ProjectileEntity.has<ProjectileComponent>() || !TargetEntity.has<HealthComponent>())
	{
		return;
	}

	const auto& Projectile = ProjectileEntity.get<ProjectileComponent>();
	if (Projectile.IsFriendly == TargetIsPlayer)
	{
		return;
	}

	auto& Health = TargetEntity.ensure<HealthComponent>();
	const int RemainingHealth = (std::max)(0, static_cast<int>(Health.HealthPercentage) - static_cast<int>(Projectile.HitPercentDamage));
	Health.HealthPercentage = static_cast<uint8_t>(RemainingHealth);
	TargetEntity.modified<HealthComponent>();

	if (RemainingHealth == 0)
	{
		MarkForDestroy(TargetEntity);
	}

	MarkForDestroy(ProjectileEntity);
}

static void BounceEnemyOffObstacle(flecs::entity Enemy)
{
	if (!Enemy.has<RigidBodyComponent>() || !Enemy.has<SpriteComponent>())
	{
		return;
	}

	auto& RigidBody = Enemy.ensure<RigidBodyComponent>();
	auto& Sprite = Enemy.ensure<SpriteComponent>();

	if (RigidBody.Velocity.x != 0)
	{
		RigidBody.Velocity.x *= -1;
		Sprite.Flip = Sprite.Flip == SDL_FLIP_NONE ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
	}

	if (RigidBody.Velocity.y != 0)
	{
		RigidBody.Velocity.y *= -1;
		Sprite.Flip = Sprite.Flip == SDL_FLIP_NONE ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
	}

	Enemy.modified<RigidBodyComponent>();
	Enemy.modified<SpriteComponent>();
}

static void CollisionDetectionSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	struct CollidableEntity
	{
		flecs::entity Entity;
		TransformComponent Transform;
		BoxColliderComponent Collider;
	};

	auto& Collision = World.get_mut<CollisionState>();
	Collision.Pairs.clear();

	std::vector<CollidableEntity> Entities;
	World.each([&Entities](flecs::entity Entity, const TransformComponent& Transform, const BoxColliderComponent& Collider)
	{
		Entities.push_back({ Entity, Transform, Collider });
	});

	for (auto A = Entities.begin(); A != Entities.end(); ++A)
	{
		for (auto B = std::next(A); B != Entities.end(); ++B)
		{
			const bool IsColliding = CheckAABBCollision
			(
				A->Transform.Position.x + A->Collider.Offset.x,
				A->Transform.Position.y + A->Collider.Offset.y,
				A->Collider.Width * A->Transform.Scale.x,
				A->Collider.Height * A->Transform.Scale.y,
				B->Transform.Position.x + B->Collider.Offset.x,
				B->Transform.Position.y + B->Collider.Offset.y,
				B->Collider.Width * B->Transform.Scale.x,
				B->Collider.Height * B->Transform.Scale.y
			);

			if (IsColliding)
			{
				Collision.Pairs.push_back({ A->Entity, B->Entity });
			}
		}
	}
}

static void CollisionResponseSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	const auto& Collision = World.get<CollisionState>();

	for (const auto& Pair : Collision.Pairs)
	{
		if (!IsAlive(World, Pair.A) || !IsAlive(World, Pair.B))
		{
			continue;
		}

		if (Pair.A.has<EnemiesTag>() && Pair.B.has<ObstaclesTag>())
		{
			BounceEnemyOffObstacle(Pair.A);
		}
		else if (Pair.B.has<EnemiesTag>() && Pair.A.has<ObstaclesTag>())
		{
			BounceEnemyOffObstacle(Pair.B);
		}

		if (Pair.A.has<ProjectilesTag>() && Pair.B.has<PlayerTag>())
		{
			HandleProjectileHitsHealthTarget(Pair.A, Pair.B, true);
		}
		else if (Pair.B.has<ProjectilesTag>() && Pair.A.has<PlayerTag>())
		{
			HandleProjectileHitsHealthTarget(Pair.B, Pair.A, true);
		}

		if (Pair.A.has<ProjectilesTag>() && Pair.B.has<EnemiesTag>())
		{
			HandleProjectileHitsHealthTarget(Pair.A, Pair.B, false);
		}
		else if (Pair.B.has<ProjectilesTag>() && Pair.A.has<EnemiesTag>())
		{
			HandleProjectileHitsHealthTarget(Pair.B, Pair.A, false);
		}
	}
}

void RegisterCollisionSystems(flecs::world& World)
{
	const auto DetectPhase = World.lookup(CollisionDetectPhaseName);
	World.system("CollisionDetectionSystem")
		.kind(DetectPhase.id())
		.each(CollisionDetectionSystemTask);

	const auto ResponsePhase = World.lookup(CollisionResponsePhaseName);
	World.system("CollisionResponseSystem")
		.kind(ResponsePhase.id())
		.each(CollisionResponseSystemTask);
}