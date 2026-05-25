#include "FlecsGameWorld.hpp"
#include "FlecsSystems.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TextLabelComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <glm/glm.hpp>

#include <algorithm>
#include <cctype>
#include <string_view>

static std::string NormalizeTag(std::string_view Tag)
{
	std::string Normalized;
	Normalized.reserve(Tag.size());
	for (char Character : Tag)
	{
		if (Character != '_' && Character != '-' && Character != ' ')
		{
			Normalized.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(Character))));
		}
	}
	return Normalized;
}

static flecs::entity CreatePhase(flecs::world& World, const char* Name, flecs::entity_t DependsOn)
{
	auto Phase = World.entity(Name);
	ecs_add_id(World.c_ptr(), Phase.id(), EcsPhase);
	ecs_add_pair(World.c_ptr(), Phase.id(), EcsDependsOn, DependsOn);
	return Phase;
}

void InputState::Clear()
{
	PressedKeys.clear();
	QuitRequested = false;
	ToggleDebugRequested = false;
}

bool InputState::WasPressed(SDL_Keycode KeyCode) const
{
	return std::find(PressedKeys.begin(), PressedKeys.end(), KeyCode) != PressedKeys.end();
}

ScriptEntity::ScriptEntity(flecs::world_t* World, flecs::entity_t EntityID)
	: World(World), EntityID(EntityID)
{
}

uint64_t ScriptEntity::GetID() const
{
	return EntityID;
}

void ScriptEntity::Destroy() const
{
	MarkForDestroy(ToEntity());
}

bool ScriptEntity::HasTag(const std::string& Tag) const
{
	auto Entity = ToEntity();
	flecs::world WorldHandle(World);
	return HasGameplayTag(WorldHandle, Entity, Tag);
}

bool ScriptEntity::BelongsToGroup(const std::string& Group) const
{
	return HasTag(Group);
}

flecs::entity ScriptEntity::ToEntity() const
{
	if (!World || EntityID == 0)
	{
		return flecs::entity();
	}

	return flecs::entity(World, EntityID);
}

void RegisterFlecsGameWorld(flecs::world& World)
{
	World.component<TransformComponent>("TransformComponent");
	World.component<RigidBodyComponent>("RigidBodyComponent");
	World.component<SpriteComponent>("SpriteComponent");
	World.component<AnimationComponent>("AnimationComponent");
	World.component<BoxColliderComponent>("BoxColliderComponent");
	World.component<HealthComponent>("HealthComponent");
	World.component<ProjectileEmitterComponent>("ProjectileEmitterComponent");
	World.component<ProjectileComponent>("ProjectileComponent");
	RegisterScriptComponents(World);
	World.component<KeyboardControlComponent>("KeyboardControlComponent");
	World.component<CameraFollowComponent>("CameraFollowComponent");
	World.component<TextLabelComponent>("TextLabelComponent");

	World.component<PlayerTag>("Player");
	World.component<EnemiesTag>("Enemies");
	World.component<ObstaclesTag>("Obstacles");
	World.component<ProjectilesTag>("Projectiles");
	World.component<TilesTag>("Tiles");
	World.component<UiTag>("Ui");
	World.component<PendingDestroyTag>("PendingDestroy");

	World.component<GameContext>("GameContext");
	World.component<InputState>("InputState");
	World.component<MapBounds>("MapBounds");
	World.component<CollisionState>("CollisionState");

	flecs::entity_t PreviousPhase = EcsOnUpdate;
	PreviousPhase = CreatePhase(World, InputPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, MovementPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, ProjectilePhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, AnimationPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, CollisionDetectPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, CollisionResponsePhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, CameraPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, ScriptPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, RenderBeginPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, RenderWorldPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, RenderUiPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, RenderDebugPhaseName, PreviousPhase).id();
	PreviousPhase = CreatePhase(World, RenderEndPhaseName, PreviousPhase).id();
	CreatePhase(World, CleanupPhaseName, PreviousPhase);
}

void RegisterFlecsSystems(flecs::world& World)
{
	RegisterInputSystems(World);
	RegisterKeyboardControlSystems(World);
	RegisterMovementSystems(World);
	RegisterProjectileSystems(World);
	RegisterAnimationSystems(World);
	RegisterCollisionSystems(World);
	RegisterCameraSystems(World);
	RegisterScriptSystems(World);
	RegisterRenderSystems(World);
	RegisterCleanupSystems(World);
}

void ApplyGameplayTag(flecs::world& World, flecs::entity Entity, const std::string& Tag)
{
	const std::string Normalized = NormalizeTag(Tag);
	if (Normalized == "player")
	{
		Entity.add<PlayerTag>();
	}
	else if (Normalized == "enemies")
	{
		Entity.add<EnemiesTag>();
	}
	else if (Normalized == "obstacles")
	{
		Entity.add<ObstaclesTag>();
	}
	else if (Normalized == "projectiles")
	{
		Entity.add<ProjectilesTag>();
	}
	else if (Normalized == "tiles")
	{
		Entity.add<TilesTag>();
	}
	else if (Normalized == "ui")
	{
		Entity.add<UiTag>();
	}
	else if (!Tag.empty())
	{
		auto DynamicTag = World.entity(Tag.c_str());
		ecs_add_id(World.c_ptr(), Entity.id(), DynamicTag.id());
	}
}

bool HasGameplayTag(flecs::world& World, flecs::entity Entity, const std::string& Tag)
{
	const std::string Normalized = NormalizeTag(Tag);
	if (Normalized == "player")
	{
		return Entity.has<PlayerTag>();
	}
	if (Normalized == "enemies")
	{
		return Entity.has<EnemiesTag>();
	}
	if (Normalized == "obstacles")
	{
		return Entity.has<ObstaclesTag>();
	}
	if (Normalized == "projectiles")
	{
		return Entity.has<ProjectilesTag>();
	}
	if (Normalized == "tiles")
	{
		return Entity.has<TilesTag>();
	}
	if (Normalized == "ui")
	{
		return Entity.has<UiTag>();
	}

	const auto DynamicTag = World.lookup(Tag.c_str());
	return DynamicTag.id() != 0 && ecs_has_id(World.c_ptr(), Entity.id(), DynamicTag.id());
}

void MarkForDestroy(flecs::entity Entity)
{
	if (Entity.id() != 0 && !Entity.has<PendingDestroyTag>())
	{
		Entity.add<PendingDestroyTag>();
	}
}

flecs::entity SpawnProjectile(flecs::world& World, const glm::vec2& Position, const glm::vec2& Velocity, const ProjectileEmitterComponent& Emitter)
{
	auto Projectile = World.entity();
	Projectile.add<ProjectilesTag>();
	Projectile.set<TransformComponent>(TransformComponent(Position, glm::vec2(1.0f, 1.0f), 0.0));
	Projectile.set<RigidBodyComponent>(RigidBodyComponent(Velocity));
	Projectile.set<SpriteComponent>(SpriteComponent("bullet-texture", 4, 4, 4));
	Projectile.set<BoxColliderComponent>(BoxColliderComponent(4, 4));
	Projectile.set<ProjectileComponent>(ProjectileComponent(Emitter.IsFriendly, Emitter.HitPercentDamage, Emitter.ProjectileDuration));
	return Projectile;
}
