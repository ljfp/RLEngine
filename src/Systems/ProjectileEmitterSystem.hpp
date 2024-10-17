#pragma once

#include <flecs.h>
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

class ProjectileEmitterSystem
{
public:
    ProjectileEmitterSystem(flecs::world& ecs)
    {
        ecs.system<ProjectileEmitterComponent, TransformComponent>()
            .each([this](flecs::entity e, ProjectileEmitterComponent& emitter, TransformComponent& transform) {
                UpdateEntity(e, emitter, transform);
            });

        ecs.observer<KeyPressedEvent>()
            .event<KeyPressedEvent>()
            .each([this](flecs::entity e, KeyPressedEvent& event) {
                OnKeyPressed(event);
            });
    }

    void OnKeyPressed(KeyPressedEvent& event)
    {
        if (event.KeyCode == SDLK_SPACE)
        {
            spdlog::info("Space key pressed");
            auto entities = e.world().filter<ProjectileEmitterComponent, TransformComponent, CameraFollowComponent>();

            for (auto entity : entities)
            {
                const auto& emitter = entity.get<ProjectileEmitterComponent>();
                const auto& transform = entity.get<TransformComponent>();
                const auto& rigidBody = entity.get<RigidBodyComponent>();

                glm::vec2 projectilePosition = transform.Position;
                if (entity.has<SpriteComponent>())
                {
                    const auto& sprite = entity.get<SpriteComponent>();
                    projectilePosition.x += (transform.Scale.x * sprite.Width / 2);
                    projectilePosition.y += (transform.Scale.y * sprite.Height / 2);
                }

                glm::vec2 projectileVelocity = emitter.ProjectileVelocity;
                int16_t directionX = 0;
                int16_t directionY = 0;
                if (rigidBody.Velocity.x > 0) directionX = +1;
                if (rigidBody.Velocity.x < 0) directionX = -1;
                if (rigidBody.Velocity.y > 0) directionY = +1;
                if (rigidBody.Velocity.y < 0) directionY = -1;
                projectileVelocity.x = directionX * emitter.ProjectileVelocity.x;
                projectileVelocity.y = directionY * emitter.ProjectileVelocity.y;

                flecs::entity projectile = e.world().entity();
                projectile.add<TransformComponent>()
                    .set<TransformComponent>({projectilePosition, glm::vec2(1.0, 1.0), 0.0});
                projectile.add<RigidBodyComponent>()
                    .set<RigidBodyComponent>({projectileVelocity});
                projectile.add<SpriteComponent>()
                    .set<SpriteComponent>({"bullet-texture", 4, 4, 4});
                projectile.add<BoxColliderComponent>()
                    .set<BoxColliderComponent>({4, 4});
                projectile.add<ProjectileComponent>()
                    .set<ProjectileComponent>({emitter.IsFriendly, emitter.HitPercentDamage, emitter.ProjectileDuration});
            }
        }
    }

    void UpdateEntity(flecs::entity e, ProjectileEmitterComponent& emitter, TransformComponent& transform)
    {
        if (emitter.ProjectileFrequency == 0) return;

        if (SDL_GetTicks() - emitter.LastEmissionTime > emitter.ProjectileFrequency)
        {
            glm::vec2 projectilePosition = transform.Position;
            if (e.has<SpriteComponent>())
            {
                const auto& sprite = e.get<SpriteComponent>();
                projectilePosition.x += (transform.Scale.x * sprite.Width / 2);
                projectilePosition.y += (transform.Scale.y * sprite.Height / 2);
            }

            flecs::entity projectile = e.world().entity();
            projectile.add<TransformComponent>()
                .set<TransformComponent>({projectilePosition, glm::vec2(1.0, 1.0), 0.0});
            projectile.add<RigidBodyComponent>()
                .set<RigidBodyComponent>({emitter.ProjectileVelocity});
            projectile.add<SpriteComponent>()
                .set<SpriteComponent>({"bullet-texture", 4, 4, 4});
            projectile.add<BoxColliderComponent>()
                .set<BoxColliderComponent>({4, 4});
            projectile.add<ProjectileComponent>()
                .set<ProjectileComponent>({emitter.IsFriendly, emitter.HitPercentDamage, emitter.ProjectileDuration});

            emitter.LastEmissionTime = SDL_GetTicks();
        }
    }
};
