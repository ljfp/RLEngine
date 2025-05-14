#pragma once

#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../ECS/FlecsBridge.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/KeyPressedEvent.hpp"
#include <spdlog/spdlog.h>

class KeyboardControlSystemFlecs : public FlecsBridgeSystem
{
public:
    KeyboardControlSystemFlecs() = default;

    void SubscribeToEvents(std::unique_ptr<EventBus>& EventBus)
    {
        EventBus->SubscribeToEvent<KeyPressedEvent>(this, &KeyboardControlSystemFlecs::OnKeyPressed);
    }

    void OnKeyPressed(KeyPressedEvent& Event)
    {
        if (!m_world) {
            spdlog::error("KeyboardControlSystemFlecs: World pointer is null!");
            return;
        }

        // Query entities with keyboard control, rigid body, and sprite components
        m_world->each([&](flecs::entity entity, 
                      KeyboardControlComponent& keyboardControl,
                      RigidBodyComponent& rigidBody, 
                      SpriteComponent& sprite) 
        {
            switch (Event.KeyCode)
            {
                // The numbers correspond with the position of the sprite in the sprite sheet.
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
        });
    }

    void Run(flecs::iter& it) override {
        // We don't use this method, as our system is event-driven
    }

    void SetupQuery(flecs::world& world, flecs::system& system) override {
        // Store the world reference
        m_world = &world;
        spdlog::info("KeyboardControlSystemFlecs: World reference set in SetupQuery");
    }
}; 