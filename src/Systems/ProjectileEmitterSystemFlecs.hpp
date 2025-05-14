#pragma once

#include "../ECS/FlecsBridge.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/KeyPressedEvent.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/ProjectileComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <spdlog/spdlog.h>

class ProjectileEmitterSystemFlecs : public FlecsBridgeSystemT<ProjectileEmitterComponent, TransformComponent>
{
public:
    ProjectileEmitterSystemFlecs() = default;
    
    // Store a reference to the world for use in methods
    void SetWorld(flecs::world* world) {
        m_world = world;
    }

    void SubscribeToEvents(std::unique_ptr<EventBus>& EventBus)
    {
        EventBus->SubscribeToEvent<KeyPressedEvent>(this, &ProjectileEmitterSystemFlecs::OnKeyPressed);
    }

    void OnKeyPressed(KeyPressedEvent& Event)
    {
        if (Event.KeyCode != SDLK_SPACE || !m_world) {
            return;
        }
        
        spdlog::info("Space key pressed");
        
        // Find entities with required components
        m_world->each([this](flecs::entity e, 
                          ProjectileEmitterComponent& emitter,
                          TransformComponent& transform)
        {
            // Only process entities with CameraFollowComponent
            if (!e.has<CameraFollowComponent>()) {
                return;
            }
            
            // Get RigidBody if available
            const RigidBodyComponent* rigidBody = e.get<RigidBodyComponent>();
            
            // Projectile position
            glm::vec2 projectilePosition = transform.Position;
            if (e.has<SpriteComponent>()) {
                const auto* sprite = e.get<SpriteComponent>();
                projectilePosition.x += (transform.Scale.x * sprite->Width / 2);
                projectilePosition.y += (transform.Scale.y * sprite->Height / 2);
            }

            // If the parent entity direction is controlled by the keyboard modify the direction of the projectile accordingly
            glm::vec2 projectileVelocity = emitter.ProjectileVelocity;
            int16_t directionX = 0;
            int16_t directionY = 0;
            
            if (rigidBody) {
                if (rigidBody->Velocity.x > 0) directionX = +1;
                if (rigidBody->Velocity.x < 0) directionX = -1;
                if (rigidBody->Velocity.y > 0) directionY = +1;
                if (rigidBody->Velocity.y < 0) directionY = -1;
                projectileVelocity.x = directionX * emitter.ProjectileVelocity.x;
                projectileVelocity.y = directionY * emitter.ProjectileVelocity.y;
            }

            // Create projectile entity
            flecs::entity projectile = m_world->entity();
            projectile.add(flecs::ChildOf, m_world->entity("Projectiles"));
            
            // Create components first
            SpriteComponent sprite;
            sprite.AssetID = "bullet-texture";
            sprite.Width = 4;
            sprite.Height = 4;
            sprite.ZIndex = 4;
            sprite.IsFixed = false;
            sprite.SrcRect = {0, 0, 4, 4};
            
            BoxColliderComponent collider;
            collider.Width = 4;
            collider.Height = 4;
            collider.Offset = glm::vec2(0.0, 0.0);
            
            ProjectileComponent projectileComp;
            projectileComp.IsFriendly = emitter.IsFriendly;
            projectileComp.HitPercentDamage = emitter.HitPercentDamage;
            projectileComp.Duration = emitter.ProjectileDuration;
            projectileComp.StartTime = SDL_GetTicks64();
            
            // Add components with proper initialization
            projectile.set<TransformComponent>({projectilePosition, glm::vec2(1.0, 1.0), 0.0});
            projectile.set<RigidBodyComponent>({projectileVelocity});
            projectile.set<SpriteComponent>(sprite);
            projectile.set<BoxColliderComponent>(collider);
            projectile.set<ProjectileComponent>(projectileComp);
        });
    }

    void Run(flecs::iter& it) override
    {
        auto emitters = it.field<ProjectileEmitterComponent>(1);
        auto transforms = it.field<TransformComponent>(2);
        
        // Update projectile emitters
        for (auto i : it) {
            auto& emitter = emitters[i];
            const auto& transform = transforms[i];
            
            // If emission frequency is zero, bypass re-emission logic
            if (emitter.ProjectileFrequency == 0) continue;
            
            // Check if it's time to re-emit a new projectile
            if (SDL_GetTicks64() - emitter.LastEmissionTime > emitter.ProjectileFrequency) {
                flecs::entity entity = it.entity(i);
                flecs::world world = it.world(); // Copy the world, don't try to reference it
                
                // Projectile position
                glm::vec2 projectilePosition = transform.Position;
                if (entity.has<SpriteComponent>()) {
                    const auto* sprite = entity.get<SpriteComponent>();
                    projectilePosition.x += (transform.Scale.x * sprite->Width / 2);
                    projectilePosition.y += (transform.Scale.y * sprite->Height / 2);
                }
                
                // Create projectile entity
                flecs::entity projectile = world.entity();
                projectile.add(flecs::ChildOf, world.entity("Projectiles"));
                
                // Create components first
                SpriteComponent sprite;
                sprite.AssetID = "bullet-texture";
                sprite.Width = 4;
                sprite.Height = 4;
                sprite.ZIndex = 4;
                sprite.IsFixed = false;
                sprite.SrcRect = {0, 0, 4, 4};
                
                BoxColliderComponent collider;
                collider.Width = 4;
                collider.Height = 4;
                collider.Offset = glm::vec2(0.0, 0.0);
                
                ProjectileComponent projectileComp;
                projectileComp.IsFriendly = emitter.IsFriendly;
                projectileComp.HitPercentDamage = emitter.HitPercentDamage;
                projectileComp.Duration = emitter.ProjectileDuration;
                projectileComp.StartTime = SDL_GetTicks64();
                
                // Add components with proper initialization
                projectile.set<TransformComponent>({projectilePosition, glm::vec2(1.0, 1.0), 0.0});
                projectile.set<RigidBodyComponent>({emitter.ProjectileVelocity});
                projectile.set<SpriteComponent>(sprite);
                projectile.set<BoxColliderComponent>(collider);
                projectile.set<ProjectileComponent>(projectileComp);
                
                // Update LastEmissionTime
                emitter.LastEmissionTime = SDL_GetTicks64();
            }
        }
    }

    void Update(std::unique_ptr<FlecsBridge>& Registry)
    {
        // Store the world reference
        m_world = &Registry->GetWorld();
        
        // This method remains for compatibility
        // The actual update happens in the Run method
    }
    
private:
    // Store a pointer to the Flecs world
    flecs::world* m_world = nullptr;
}; 