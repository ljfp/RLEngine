#pragma once

#include <flecs.h>
#include "../Components/TransformComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Game/Game.hpp"
#include <spdlog/spdlog.h>

// This system shows how to use Flecs directly without the bridge
// It's a direct implementation of MovementSystem using pure Flecs API
namespace DirectFlecsSystem {

// Register components with Flecs
void RegisterComponents(flecs::world& world) {
    world.component<TransformComponent>()
        .member<glm::vec2>("Position")
        .member<glm::vec2>("Scale")
        .member<double>("Rotation");

    world.component<RigidBodyComponent>()
        .member<glm::vec2>("Velocity");
}

// Create a movement system using pure Flecs API
void CreateMovementSystem(flecs::world& world) {
    // Create a system that processes TransformComponent and RigidBodyComponent
    auto sys = world.system<TransformComponent, RigidBodyComponent>();
    sys.kind(flecs::OnUpdate);
    sys.each([](flecs::iter& it, size_t i, TransformComponent& transform, RigidBodyComponent& rigidBody) {
        // Access to delta time through the iterator
        float dt = it.delta_time();
        
        transform.Position.x += rigidBody.Velocity.x * dt;
        transform.Position.y += rigidBody.Velocity.y * dt;
        
        flecs::entity e = it.entity(i);
        
        // Prevent the player from moving outside the map.
        if (e.has(it.world().entity("Player"))) {
            uint8_t PaddingLeft = 10;
            uint8_t PaddingTop = 10;
            uint8_t PaddingRight = 50;
            uint8_t PaddingBottom = 50;
            transform.Position.x = transform.Position.x < PaddingLeft ? PaddingLeft : transform.Position.x;
            transform.Position.x = transform.Position.x > Game::MapWidth - PaddingRight ? Game::MapWidth - PaddingRight : transform.Position.x;
            transform.Position.y = transform.Position.y < PaddingTop ? PaddingTop : transform.Position.y;
            transform.Position.y = transform.Position.y > Game::MapHeight - PaddingBottom ? Game::MapHeight - PaddingBottom : transform.Position.y;
        }

        uint8_t Padding = 100; // In pixels
        bool IsEntityOutsideOfBounds =
        (
            transform.Position.x < -Padding ||
            transform.Position.x > Game::MapWidth + Padding ||
            transform.Position.y < -Padding ||
            transform.Position.y > Game::MapHeight + Padding
        );

        // Kill all entities that move outside of the map boundaries.
        if (IsEntityOutsideOfBounds && !e.has(it.world().entity("Player"))) {
            e.destruct();
        }
    });
}

// Example of creating a system with a query filter
void CreatePlayerMovementSystem(flecs::world& world) {
    // Create Player tag first
    flecs::entity playerTag = world.entity("Player");
    
    // This system only processes entities that have the "Player" tag
    auto sys = world.system<TransformComponent, RigidBodyComponent>();
    sys.with(playerTag); // Filter to only match entities with Player tag
    sys.each([](flecs::iter& it, size_t i, TransformComponent& transform, RigidBodyComponent& rigidBody) {
        // Process player movement with special rules
        spdlog::info("Processing player movement at position: ({}, {})", 
                    transform.Position.x, transform.Position.y);
        
        // Player-specific movement logic could go here
    });
}

// Initialize all direct Flecs systems
void RegisterSystems(flecs::world& world) {
    RegisterComponents(world);
    CreateMovementSystem(world);
    CreatePlayerMovementSystem(world);
    
    spdlog::info("Direct Flecs systems registered");
}

} 