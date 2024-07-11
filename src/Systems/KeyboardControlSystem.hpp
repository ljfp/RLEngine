#pragma once

#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../ECS/ECS.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/KeyPressedEvent.hpp"
#include <spdlog/spdlog.h>

class KeyboardControlSystem : public System
{
public:
	KeyboardControlSystem()
	{
		RequireComponent<KeyboardControlComponent>();
		RequireComponent<RigidBodyComponent>();
		RequireComponent<SpriteComponent>();
	}

	void SubscribeToEvents(std::unique_ptr<EventBus>& EventBus)
	{
		EventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystem::OnKeyPressed);
	}

	void OnKeyPressed(KeyPressedEvent& Event)
	{
		//std::string KeySymbol(1, Event.KeyCode);
		//spdlog::info("Key pressed event emitted with key code {} and key symbol {}", Event.KeyCode, KeySymbol);

		for (auto AnEntity : GetSystemEntities())
		{
			const auto KeyboardControl = AnEntity.GetComponent<KeyboardControlComponent>();
			auto& RigidBody = AnEntity.GetComponent<RigidBodyComponent>();
			auto& Sprite = AnEntity.GetComponent<SpriteComponent>();

			switch (Event.KeyCode)
			{
				// The numbers correspond with the position of the sprite in the sprite sheet.
				case SDLK_UP:
					RigidBody.Velocity = KeyboardControl.UpVelocity;
					Sprite.SrcRect.y = Sprite.Height * 0;
					break;
				case SDLK_DOWN:
					RigidBody.Velocity = KeyboardControl.DownVelocity;
					Sprite.SrcRect.y = Sprite.Height * 2;
					break;
				case SDLK_LEFT:
					RigidBody.Velocity = KeyboardControl.LeftVelocity;
					Sprite.SrcRect.y = Sprite.Height * 3;
					break;
				case SDLK_RIGHT:
					RigidBody.Velocity = KeyboardControl.RightVelocity;
					Sprite.SrcRect.y = Sprite.Height * 1;
					break;
			}
		}
	}
};