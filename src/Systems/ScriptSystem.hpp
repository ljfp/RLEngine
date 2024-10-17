#pragma once

#include <flecs.h>
#include "../Components/AnimationComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/ScriptComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include <spdlog/spdlog.h>
#include <tuple>

// We first declare native C++ functions that we want to bind with Lua functions.
std::tuple<double, double> GetEntityPosition(flecs::entity AnEntity)
{
	if (AnEntity.has<TransformComponent>())
	{
		const auto& Transform = AnEntity.get<TransformComponent>();
		return std::make_tuple(Transform.Position.x, Transform.Position.y);
	}
	else
	{
		spdlog::error("Entity does not have a TransformComponent.");
		return std::make_tuple(0.0, 0.0);
	}
}

std::tuple<double, double> GetEntityVelocity(flecs::entity AnEntity)
{
	if (AnEntity.has<RigidBodyComponent>())
	{
		const auto& RigidBody = AnEntity.get<RigidBodyComponent>();
		return std::make_tuple(RigidBody.Velocity.x, RigidBody.Velocity.y);
	}
	else
	{
		spdlog::error("Entity does not have a RigidBodyComponent.");
		return std::make_tuple(0.0, 0.0);
	}
}

void SetEntityPosition(flecs::entity AnEntity, double x, double y)
{
	if (AnEntity.has<TransformComponent>())
	{
		auto& Transform = AnEntity.get_mut<TransformComponent>();
		Transform.Position = glm::vec2(x, y);
	}
	else
	{
		spdlog::error("Entity does not have a TransformComponent.");
	}
}

void SetEntityVelocity(flecs::entity AnEntity, double x, double y)
{
	if (AnEntity.has<RigidBodyComponent>())
	{
		auto& RigidBody = AnEntity.get_mut<RigidBodyComponent>();
		RigidBody.Velocity = glm::vec2(x, y);
	}
	else
	{
		spdlog::error("Entity does not have a RigidBodyComponent.");
	}
}

void SetEntityRotation(flecs::entity AnEntity, double Angle)
{
	if (AnEntity.has<TransformComponent>())
	{
		auto& Transform = AnEntity.get_mut<TransformComponent>();
		Transform.Rotation = Angle;
	}
	else
	{
		spdlog::error("Entity does not have a TransformComponent.");
	}
}

void SetEntityAnimationFrame(flecs::entity AnEntity, uint8_t Frame)
{
	if (AnEntity.has<AnimationComponent>())
	{
		auto& Animation = AnEntity.get_mut<AnimationComponent>();
		Animation.CurrentFrame = Frame;
	}
	else
	{
		spdlog::error("Entity does not have an AnimationComponent.");
	}
}

void SetProjectileVelocity(flecs::entity AnEntity, double x, double y)
{
	if (AnEntity.has<ProjectileEmitterComponent>())
	{
		auto& ProjectileEmitter = AnEntity.get_mut<ProjectileEmitterComponent>();
		ProjectileEmitter.ProjectileVelocity = glm::vec2(x, y);
	}
	else
	{
		spdlog::error("Entity does not have a ProjectileEmitterComponent.");
	}
}

class ScriptSystem
{
public:
	ScriptSystem(flecs::world& ecs)
	{
		ecs.system<ScriptComponent>()
			.each([this](flecs::entity e, ScriptComponent& script) {
				this->entities.push_back(e);
			});
	}

	void CreateLuaBindings(sol::state& LuaState)
	{
		// Create the "Entity" class in Lua.
		LuaState.new_usertype<flecs::entity>
		(
			"entity",
			"get_id", &flecs::entity::id,
			"destroy", &flecs::entity::destruct,
			"has_tag", &flecs::entity::has_tag,
			"belongs_to_group", &flecs::entity::belongs_to_group
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
		for (auto e : entities)
		{
			const auto& script = e.get<ScriptComponent>();
			script.Funct(e, DeltaTime, EllapsedTime);
		}
	}

private:
	std::vector<flecs::entity> entities;
};
