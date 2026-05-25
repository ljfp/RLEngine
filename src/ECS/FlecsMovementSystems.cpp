#include "FlecsSystems.hpp"
#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3/SDL.h>
#include <cstdint>

static void KeyboardControlSystemTask(flecs::iter& Iter, size_t, const KeyboardControlComponent& KeyboardControl, RigidBodyComponent& RigidBody, SpriteComponent& Sprite)
{
	const auto& Input = Iter.world().get<InputState>();
	for (const SDL_Keycode KeyCode : Input.PressedKeys)
	{
		switch (KeyCode)
		{
		case SDLK_UP:
			RigidBody.Velocity = KeyboardControl.UpVelocity;
			Sprite.SrcRect.y = Sprite.Height * 0.0f;
			break;
		case SDLK_DOWN:
			RigidBody.Velocity = KeyboardControl.DownVelocity;
			Sprite.SrcRect.y = Sprite.Height * 2.0f;
			break;
		case SDLK_LEFT:
			RigidBody.Velocity = KeyboardControl.LeftVelocity;
			Sprite.SrcRect.y = Sprite.Height * 3.0f;
			break;
		case SDLK_RIGHT:
			RigidBody.Velocity = KeyboardControl.RightVelocity;
			Sprite.SrcRect.y = Sprite.Height * 1.0f;
			break;
		default:
			break;
		}
	}
}

static void MovementSystemTask(flecs::iter& Iter, size_t Row, TransformComponent& Transform, const RigidBodyComponent& RigidBody)
{
	auto World = Iter.world();
	flecs::entity Entity = Iter.entity(Row);
	const auto& Bounds = World.get<MapBounds>();
	Transform.Position.x += RigidBody.Velocity.x * Iter.delta_time();
	Transform.Position.y += RigidBody.Velocity.y * Iter.delta_time();

	if (Entity.has<PlayerTag>())
	{
		constexpr uint8_t PaddingLeft = 10;
		constexpr uint8_t PaddingTop = 10;
		constexpr uint8_t PaddingRight = 50;
		constexpr uint8_t PaddingBottom = 50;

		Transform.Position.x = Transform.Position.x < PaddingLeft ? PaddingLeft : Transform.Position.x;
		Transform.Position.x = Transform.Position.x > Bounds.Width - PaddingRight ? Bounds.Width - PaddingRight : Transform.Position.x;
		Transform.Position.y = Transform.Position.y < PaddingTop ? PaddingTop : Transform.Position.y;
		Transform.Position.y = Transform.Position.y > Bounds.Height - PaddingBottom ? Bounds.Height - PaddingBottom : Transform.Position.y;
	}

	constexpr uint8_t Padding = 100;
	const bool IsOutsideBounds =
		Transform.Position.x < -Padding ||
		Transform.Position.x > Bounds.Width + Padding ||
		Transform.Position.y < -Padding ||
		Transform.Position.y > Bounds.Height + Padding;

	if (IsOutsideBounds && !Entity.has<PlayerTag>())
	{
		MarkForDestroy(Entity);
	}
}

void RegisterKeyboardControlSystems(flecs::world& World)
{
	const auto Phase = World.lookup(InputPhaseName);
	World.system<KeyboardControlComponent, RigidBodyComponent, SpriteComponent>("KeyboardControlSystem")
		.kind(Phase.id())
		.each(KeyboardControlSystemTask);
}

void RegisterMovementSystems(flecs::world& World)
{
	const auto Phase = World.lookup(MovementPhaseName);
	World.system<TransformComponent, const RigidBodyComponent>("MovementSystem")
		.kind(Phase.id())
		.each(MovementSystemTask);
}