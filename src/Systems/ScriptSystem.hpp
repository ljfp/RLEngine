#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/ScriptComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include <spdlog/spdlog.h>
#include <tuple>

// We first declare native C++ functions that we want to bind with Lua functions.
std::tuple<double, double> GetEntityPosition(Entity AnEntity)
{
	if (AnEntity.HasComponent<TransformComponent>())
	{
		const auto Transform = AnEntity.GetComponent<TransformComponent>();
		return std::make_tuple(Transform.Position.x, Transform.Position.y);
	}
	else
	{
		spdlog::error("Entity does not have a TransformComponent.");
		return std::make_tuple(0.0, 0.0);
	}
}

std::tuple<double, double> GetEntityVelocity(Entity AnEntity)
{
	if (AnEntity.HasComponent<RigidBodyComponent>())
	{
		const auto RigidBody = AnEntity.GetComponent<RigidBodyComponent>();
		return std::make_tuple(RigidBody.Velocity.x, RigidBody.Velocity.y);
	}
	else
	{
		spdlog::error("Entity does not have a RigidBodyComponent.");
		return std::make_tuple(0.0, 0.0);
	}
}

void SetEntityPosition(Entity AnEntity, double x, double y)
{
	// TODO:
	if (AnEntity.HasComponent<TransformComponent>())
	{
		auto& Transform = AnEntity.GetComponent<TransformComponent>();
		Transform.Position = glm::vec2(x, y);
	}
	else
	{
		spdlog::error("Entity does not have a TransformComponent.");
	}
}

void SetEntityVelocity(Entity AnEntity, double x, double y)
{
	if (AnEntity.HasComponent<RigidBodyComponent>())
	{
		auto& RigidBody = AnEntity.GetComponent<RigidBodyComponent>();
		RigidBody.Velocity = glm::vec2(x, y);
	}
	else
	{
		spdlog::error("Entity does not have a RigidBodyComponent.");
	}
}

void SetEntityRotation(Entity AnEntity, double Angle)
{
	if (AnEntity.HasComponent<TransformComponent>())
	{
		auto& Transform = AnEntity.GetComponent<TransformComponent>();
		Transform.Rotation = Angle;
	}
	else
	{
		spdlog::error("Entity does not have a TransformComponent.");
	}
}

void SetEntityAnimationFrame(Entity AnEntity, uint8_t Frame)
{
	if (AnEntity.HasComponent<AnimationComponent>())
	{
		auto& Animation = AnEntity.GetComponent<AnimationComponent>();
		Animation.CurrentFrame = Frame;
	}
	else
	{
		spdlog::error("Entity does not have an AnimationComponent.");
	}
}

void SetProjectileVelocity(Entity AnEntity, double x, double y)
{
	if (AnEntity.HasComponent<ProjectileEmitterComponent>())
	{
		auto& ProjectileEmitter = AnEntity.GetComponent<ProjectileEmitterComponent>();
		ProjectileEmitter.ProjectileVelocity = glm::vec2(x, y);
	}
	else
	{
		spdlog::error("Entity does not have a ProjectileEmitterComponent.");
	}
}

class ScriptSystem : public System
{
public:
	ScriptSystem()
	{
		RequireComponent<ScriptComponent>();
	}

	void CreateLuaBindings(sol::state& LuaState)
	{
		// Create the "Entity" class in Lua.
		LuaState.new_usertype<Entity>
		(
			"entity",
			"get_id", &Entity::GetID,
			"destroy", &Entity::Kill,
			"has_tag", &Entity::HasTag,
			"belongs_to_group", &Entity::BelongsToGroup
		);

		LuaState.set_function("get_position", GetEntityPosition);
		LuaState.set_function("get_velocity", GetEntityVelocity);
		LuaState.set_function("set_position", SetEntityPosition);
		LuaState.set_function("set_velocity", SetEntityVelocity);
		LuaState.set_function("set_rotation", SetEntityRotation);
		LuaState.set_function("set_projectile_velocity", SetProjectileVelocity);
		LuaState.set_function("set_animation_frame", SetEntityAnimationFrame);
	}

	void Update(double DeltaTime, uint64_t EllapsedTime)
	{
		for (auto AnEntity : GetSystemEntities())
		{
			const auto Script = AnEntity.GetComponent<ScriptComponent>();
			Script.Funct(AnEntity, DeltaTime, EllapsedTime);
		}
	}
};