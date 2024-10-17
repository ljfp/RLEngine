#pragma once

#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include <flecs.h>
#include "../EventBus/EventBus.hpp"
#include "../Events/KeyPressedEvent.hpp"
#include <spdlog/spdlog.h>

class KeyboardControlSystem
{
public:
	KeyboardControlSystem(flecs::world& ecs)
	{
		ecs.system<KeyboardControlComponent, RigidBodyComponent, SpriteComponent>()
			.each([this](flecs::entity e, KeyboardControlComponent& keyboardControl, RigidBodyComponent& rigidBody, SpriteComponent& sprite) {
				this->entities.push_back(e);
			});
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& EventBus)
	{
		EventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& Event)
	{
		for (auto e : entities)
		{
			const auto& keyboardControl = e.get<KeyboardControlComponent>();
			auto& rigidBody = e.get_mut<RigidBodyComponent>();
			auto& sprite = e.get_mut<SpriteComponent>();

			switch (Event.KeyCode)
			{
				case SDLK_UP:
					rigidBody.Velocity = keyboardControl.UpVelocity;
					sprite.SrcRect.y = sprite.Height * 0;
					break;
				case SDLK_DOWN:
					rigidBody.Velocity = keyboardControl.DownVelocity;
					sprite.SrcRect.y = sprite.Height * 2;
					break;
				case SDLK_LEFT:
					rigidBody.Velocity = keyboardControl.LeftVelocity;
					sprite.SrcRect.y = sprite.Height * 3;
					break;
				case SDLK_RIGHT:
					rigidBody.Velocity = keyboardControl.RightVelocity;
					sprite.SrcRect.y = sprite.Height * 1;
					break;
			}
		}
	}

private:
	std::vector<flecs::entity> entities;
};
