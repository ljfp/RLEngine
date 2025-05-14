#pragma once

#include "../ECS/FlecsBridge.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/CollisionEvent.hpp"
#include "../Game/Game.hpp"

// Flecs version of the MovementSystem
class MovementSystemFlecs : public FlecsBridgeSystemT<TransformComponent, RigidBodyComponent>
{
public:
    MovementSystemFlecs() = default;

    void SubscribeToEvents(const std::unique_ptr<EventBus>& EventBus)
    {
        EventBus->SubscribeToEvent<CollisionEvent>(this, &MovementSystemFlecs::OnCollision);
    }

    void Run(flecs::iter& it) override
    {
        auto transforms = it.field<TransformComponent>(1);
        auto rigidBodies = it.field<RigidBodyComponent>(2);
        
        for (auto i : it) {
            transforms[i].Position.x += rigidBodies[i].Velocity.x * it.delta_time();
            transforms[i].Position.y += rigidBodies[i].Velocity.y * it.delta_time();
            
            flecs::entity entity = it.entity(i);
            
            // Prevent the player from moving outside the map.
            if (entity.has(it.world().entity("Player"))) {
                uint8_t PaddingLeft = 10;
                uint8_t PaddingTop = 10;
                uint8_t PaddingRight = 50;
                uint8_t PaddingBottom = 50;
                transforms[i].Position.x = transforms[i].Position.x < PaddingLeft ? PaddingLeft : transforms[i].Position.x;
                transforms[i].Position.x = transforms[i].Position.x > Game::MapWidth - PaddingRight ? Game::MapWidth - PaddingRight : transforms[i].Position.x;
                transforms[i].Position.y = transforms[i].Position.y < PaddingTop ? PaddingTop : transforms[i].Position.y;
                transforms[i].Position.y = transforms[i].Position.y > Game::MapHeight - PaddingBottom ? Game::MapHeight - PaddingBottom : transforms[i].Position.y;
            }

            uint8_t Padding = 100; // In pixels
            bool IsEntityOutsideOfBounds =
            (
                transforms[i].Position.x < -Padding ||
                transforms[i].Position.x > Game::MapWidth + Padding ||
                transforms[i].Position.y < -Padding ||
                transforms[i].Position.y > Game::MapHeight + Padding
            );

            // Kill all entities that move outside of the map boundaries.
            if (IsEntityOutsideOfBounds && !entity.has(it.world().entity("Player"))) {
                entity.destruct();
            }
        }
    }

    // This function is called manually during the Update method of the game
    void Update(double DeltaTime)
    {
        // The actual update happens in the Run method called by Flecs
        // This method remains for compatibility
    }

    void OnCollision(CollisionEvent& Event)
    {
        // Convert Entity to FlecsBridge::Entity
        // We need to create appropriate entities from IDs
        auto entity_id_a = Event.A.GetID();
        auto entity_id_b = Event.B.GetID();
        
        // Create Flecs entities using the entity IDs
        auto& world = *m_world;
        FlecsBridge::Entity EntityA(world.entity(entity_id_a));
        FlecsBridge::Entity EntityB(world.entity(entity_id_b));
        
        spdlog::info("The damage system received an event collision between entities {} and {}", EntityA.GetID(), EntityB.GetID());

        auto entityA = EntityA.GetFlecsEntity();
        auto entityB = EntityB.GetFlecsEntity();
        
        if (entityA.has(flecs::ChildOf, entityA.world().entity("Enemies")) && 
            entityB.has(flecs::ChildOf, entityB.world().entity("Obstacles"))) {
            OnEnemyHitsObstacle(EntityA, EntityB);
        }

        if (entityA.has(flecs::ChildOf, entityA.world().entity("Obstacles")) && 
            entityB.has(flecs::ChildOf, entityB.world().entity("Enemies"))) {
            OnEnemyHitsObstacle(EntityB, EntityA);
        }
    }

    void OnEnemyHitsObstacle(FlecsBridge::Entity Enemy, FlecsBridge::Entity Obstacle)
    {
        if (Enemy.HasComponent<RigidBodyComponent>() && Enemy.HasComponent<SpriteComponent>()) {
            auto& RigidBody = Enemy.GetComponent<RigidBodyComponent>();
            auto& Sprite = Enemy.GetComponent<SpriteComponent>();

            if (RigidBody.Velocity.x != 0) {
                RigidBody.Velocity.x *= -1;
                Sprite.Flip = (Sprite.Flip == SDL_FLIP_NONE) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
            }

            if (RigidBody.Velocity.y != 0) {
                RigidBody.Velocity.y *= -1;
                Sprite.Flip = (Sprite.Flip == SDL_FLIP_NONE) ? SDL_FLIP_VERTICAL : SDL_FLIP_NONE;
            }
        }
    }

private:
    // Store a pointer to the Flecs world
    flecs::world* m_world = nullptr;
}; 