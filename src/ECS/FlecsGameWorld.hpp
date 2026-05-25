#pragma once

#include <SDL3/SDL.h>
#include <flecs.h>
#include <glm/vec2.hpp>
#include <sol/sol.hpp>

#include <cstdint>
#include <string>
#include <vector>

class AssetManager;
struct ProjectileEmitterComponent;

struct PlayerTag {};
struct EnemiesTag {};
struct ObstaclesTag {};
struct ProjectilesTag {};
struct TilesTag {};
struct UiTag {};
struct PendingDestroyTag {};

struct GameContext
{
	SDL_Renderer* Renderer = nullptr;
	AssetManager* Assets = nullptr;
	SDL_FRect* Camera = nullptr;
	bool* IsDebug = nullptr;
	bool* IsRunning = nullptr;
};

struct InputState
{
	std::vector<SDL_Keycode> PressedKeys;
	bool QuitRequested = false;
	bool ToggleDebugRequested = false;

	void Clear();
	bool WasPressed(SDL_Keycode KeyCode) const;
};

struct MapBounds
{
	uint16_t Width = 0;
	uint16_t Height = 0;
};

struct CollisionPair
{
	flecs::entity A;
	flecs::entity B;
};

struct CollisionState
{
	std::vector<CollisionPair> Pairs;
};

struct ScriptEntity
{
	flecs::world_t* World = nullptr;
	flecs::entity_t EntityID = 0;

	ScriptEntity() = default;
	ScriptEntity(flecs::world_t* World, flecs::entity_t EntityID);

	uint64_t GetID() const;
	void Destroy() const;
	bool HasTag(const std::string& Tag) const;
	bool BelongsToGroup(const std::string& Group) const;
	flecs::entity ToEntity() const;
};

void RegisterFlecsGameWorld(flecs::world& World);
void RegisterFlecsSystems(flecs::world& World);
void RegisterScriptBindings(flecs::world& World, sol::state& LuaState);

void ApplyGameplayTag(flecs::world& World, flecs::entity Entity, const std::string& Tag);
bool HasGameplayTag(flecs::world& World, flecs::entity Entity, const std::string& Tag);
void MarkForDestroy(flecs::entity Entity);

flecs::entity SpawnProjectile
(
	flecs::world& World,
	const glm::vec2& Position,
	const glm::vec2& Velocity,
	const ProjectileEmitterComponent& Emitter
);
