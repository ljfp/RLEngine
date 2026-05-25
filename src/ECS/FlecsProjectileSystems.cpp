#include "FlecsSystems.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <cstdint>

static glm::vec2 GetProjectileOrigin(flecs::entity Entity, const TransformComponent& Transform)
{
	glm::vec2 ProjectilePosition = Transform.Position;
	if (Entity.has<SpriteComponent>())
	{
		const auto& Sprite = Entity.get<SpriteComponent>();
		ProjectilePosition.x += Transform.Scale.x * Sprite.Width / 2.0f;
		ProjectilePosition.y += Transform.Scale.y * Sprite.Height / 2.0f;
	}
	return ProjectilePosition;
}

static void ProjectileEmitterSystemTask(flecs::iter& Iter, size_t Row, ProjectileEmitterComponent& Emitter, const TransformComponent& Transform)
{
	auto World = Iter.world();
	flecs::entity Entity = Iter.entity(Row);
	const auto& Input = World.get<InputState>();

	if (Input.WasPressed(SDLK_SPACE) && Entity.has<CameraFollowComponent>() && Entity.has<RigidBodyComponent>())
	{
		const auto& RigidBody = Entity.get<RigidBodyComponent>();
		glm::vec2 ProjectileVelocity = Emitter.ProjectileVelocity;
		int16_t DirectionX = 0;
		int16_t DirectionY = 0;

		if (RigidBody.Velocity.x > 0) DirectionX = 1;
		if (RigidBody.Velocity.x < 0) DirectionX = -1;
		if (RigidBody.Velocity.y > 0) DirectionY = 1;
		if (RigidBody.Velocity.y < 0) DirectionY = -1;

		ProjectileVelocity.x = DirectionX * Emitter.ProjectileVelocity.x;
		ProjectileVelocity.y = DirectionY * Emitter.ProjectileVelocity.y;
		SpawnProjectile(World, GetProjectileOrigin(Entity, Transform), ProjectileVelocity, Emitter);
	}

	if (Emitter.ProjectileFrequency == 0)
	{
		return;
	}

	if (SDL_GetTicks() - Emitter.LastEmissionTime > Emitter.ProjectileFrequency)
	{
		SpawnProjectile(World, GetProjectileOrigin(Entity, Transform), Emitter.ProjectileVelocity, Emitter);
		Emitter.LastEmissionTime = SDL_GetTicks();
	}
}

static void ProjectileLifecycleSystemTask(flecs::iter& Iter, size_t Row, ProjectileComponent& Projectile)
{
	if (SDL_GetTicks() - Projectile.StartTime > Projectile.Duration)
	{
		MarkForDestroy(Iter.entity(Row));
	}
}

void RegisterProjectileSystems(flecs::world& World)
{
	const auto Phase = World.lookup(ProjectilePhaseName);

	World.system<ProjectileEmitterComponent, const TransformComponent>("ProjectileEmitterSystem")
		.kind(Phase.id())
		.each(ProjectileEmitterSystemTask);

	World.system<ProjectileComponent>("ProjectileLifecycleSystem")
		.kind(Phase.id())
		.each(ProjectileLifecycleSystemTask);
}