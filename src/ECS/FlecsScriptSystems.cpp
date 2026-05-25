#include "FlecsSystems.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/ScriptComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

#include <cstdint>
#include <tuple>

static std::tuple<double, double> GetEntityPosition(ScriptEntity ScriptEntity)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<TransformComponent>())
	{
		const auto& Transform = Entity.get<TransformComponent>();
		return std::make_tuple(Transform.Position.x, Transform.Position.y);
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have a TransformComponent.");
	return std::make_tuple(0.0, 0.0);
}

static std::tuple<double, double> GetEntityVelocity(ScriptEntity ScriptEntity)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<RigidBodyComponent>())
	{
		const auto& RigidBody = Entity.get<RigidBodyComponent>();
		return std::make_tuple(RigidBody.Velocity.x, RigidBody.Velocity.y);
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have a RigidBodyComponent.");
	return std::make_tuple(0.0, 0.0);
}

static void SetEntityPosition(ScriptEntity ScriptEntity, double X, double Y)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<TransformComponent>())
	{
		auto& Transform = Entity.ensure<TransformComponent>();
		Transform.Position = glm::vec2(X, Y);
		Entity.modified<TransformComponent>();
		return;
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have a TransformComponent.");
}

static void SetEntityVelocity(ScriptEntity ScriptEntity, double X, double Y)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<RigidBodyComponent>())
	{
		auto& RigidBody = Entity.ensure<RigidBodyComponent>();
		RigidBody.Velocity = glm::vec2(X, Y);
		Entity.modified<RigidBodyComponent>();
		return;
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have a RigidBodyComponent.");
}

static void SetEntityRotation(ScriptEntity ScriptEntity, double Angle)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<TransformComponent>())
	{
		auto& Transform = Entity.ensure<TransformComponent>();
		Transform.Rotation = Angle;
		Entity.modified<TransformComponent>();
		return;
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have a TransformComponent.");
}

static void SetEntityAnimationFrame(ScriptEntity ScriptEntity, uint8_t Frame)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<AnimationComponent>())
	{
		auto& Animation = Entity.ensure<AnimationComponent>();
		Animation.CurrentFrame = Frame;
		Entity.modified<AnimationComponent>();
		return;
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have an AnimationComponent.");
}

static void SetProjectileVelocity(ScriptEntity ScriptEntity, double X, double Y)
{
	auto Entity = ScriptEntity.ToEntity();
	if (Entity.has<ProjectileEmitterComponent>())
	{
		auto& ProjectileEmitter = Entity.ensure<ProjectileEmitterComponent>();
		ProjectileEmitter.ProjectileVelocity = glm::vec2(X, Y);
		Entity.modified<ProjectileEmitterComponent>();
		return;
	}

	SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Entity does not have a ProjectileEmitterComponent.");
}

static void ScriptSystemTask(flecs::iter& Iter, size_t Row, const ScriptComponent& Script)
{
	if (Script.Funct.valid())
	{
		auto World = Iter.world();
		Script.Funct(ScriptEntity(World.c_ptr(), Iter.entity(Row).id()), Iter.delta_time(), SDL_GetTicks());
	}
}

void RegisterScriptComponents(flecs::world& World)
{
	World.component<ScriptComponent>("ScriptComponent");
}

void RegisterScriptSystems(flecs::world& World)
{
	const auto Phase = World.lookup(ScriptPhaseName);
	World.system<ScriptComponent>("ScriptSystem")
		.kind(Phase.id())
		.each(ScriptSystemTask);
}

void RegisterScriptBindings(flecs::world&, sol::state& LuaState)
{
	LuaState.new_usertype<ScriptEntity>
	(
		"entity",
		"get_id", &ScriptEntity::GetID,
		"destroy", &ScriptEntity::Destroy,
		"has_tag", &ScriptEntity::HasTag,
		"belongs_to_group", &ScriptEntity::BelongsToGroup
	);

	LuaState.set_function("get_position", GetEntityPosition);
	LuaState.set_function("get_velocity", GetEntityVelocity);
	LuaState.set_function("set_position", SetEntityPosition);
	LuaState.set_function("set_velocity", SetEntityVelocity);
	LuaState.set_function("set_rotation", SetEntityRotation);
	LuaState.set_function("set_projectile_velocity", SetProjectileVelocity);
	LuaState.set_function("set_animation_frame", SetEntityAnimationFrame);
}
