#pragma once

#include "../ECS/ECS.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/ScriptComponent.hpp"
#include <spdlog/spdlog.h>

// We first declare native C++ functions that we want to bind with Lua functions.
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

		//LuaState.set_function("GetEntityPosition", GetEntityPosition);
		LuaState.set_function("set_position", SetEntityPosition);
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