#include <flecs.h>
#include <iostream>
#include <cmath>

// Example component types
struct Position {
    float x;
    float y;
};

struct Velocity {
    float x;
    float y;
};

struct Health {
    int value;
};

struct Damage {
    int value;
};

// Tag components
struct Enemy { };
struct Player { };
struct Projectile { };

int main() {
    // Create a new Flecs world
    flecs::world world;
    
    // Register components with the world (optional but recommended)
    world.component<Position>()
        .member<float>("x")
        .member<float>("y");
        
    world.component<Velocity>()
        .member<float>("x")
        .member<float>("y");
        
    world.component<Health>()
        .member<int>("value");
        
    world.component<Damage>()
        .member<int>("value");
    
    // Create a player entity
    auto player = world.entity("Player")
        .set<Position>({100, 100})
        .set<Velocity>({0, 0})
        .set<Health>({100})
        .add<Player>();
        
    // Create an enemy entity
    auto enemy = world.entity("Enemy")
        .set<Position>({200, 200})
        .set<Velocity>({-1, 0})
        .set<Health>({50})
        .set<Damage>({10})
        .add<Enemy>();
        
    // Create prefab for projectiles
    auto projectilePrefab = world.prefab("ProjectilePrefab")
        .set<Velocity>({5, 0})
        .set<Damage>({25})
        .add<Projectile>();
        
    // Create a projectile from the prefab
    auto projectile = world.entity()
        .is_a(projectilePrefab)
        .set<Position>({100, 100});
        
    // Create a movement system
    world.system<Position, const Velocity>()
        .each([](Position& p, const Velocity& v) {
            p.x += v.x;
            p.y += v.y;
        });
        
    // Create a collision system with a more basic approach
    world.system<Position, Health>()
        .each([&world](flecs::entity entity, Position& position, Health& health) {
            // Only check entities that have health (potential targets)
            world.each([&](flecs::entity projectile) {
                if (!projectile.has<Projectile>() || !projectile.has<Position>() || !projectile.has<Damage>()) {
                    return true; // Continue to next entity
                }
                
                auto& projectilePos = *projectile.get<Position>();
                auto& damage = *projectile.get<Damage>();
                
                // Simple distance check for collision
                float dx = position.x - projectilePos.x;
                float dy = position.y - projectilePos.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance < 10.0f) {  // Collision radius
                    health.value -= damage.value;
                    std::cout << "Entity " << (entity.name() ? entity.name() : "unnamed") 
                              << " hit by projectile! Health: " << health.value << std::endl;
                        
                    // Destroy projectile after hit
                    projectile.destruct();
                    
                    // Check if entity is destroyed
                    if (health.value <= 0) {
                        std::cout << "Entity " << (entity.name() ? entity.name() : "unnamed") 
                                  << " destroyed!" << std::endl;
                        entity.destruct();
                        return false; // Stop iteration after entity is destroyed
                    }
                }
                
                return true; // Continue to next entity
            });
        });
        
    // Create an observer that triggers when an entity's health changes
    world.observer<Health>()
        .event(flecs::OnSet)
        .each([](flecs::entity e, Health& health) {
            std::cout << "Health of entity " << (e.name() ? e.name() : "unnamed") 
                      << " changed to " << health.value << std::endl;
        });
        
    // Run several frames of the simulation
    for (int i = 0; i < 10; i++) {
        std::cout << "------ Frame " << i << " ------" << std::endl;
        world.progress();  // This calls all systems
        
        // Create a new projectile every few frames
        if (i % 3 == 0) {
            auto newProjectile = world.entity()
                .is_a(projectilePrefab)
                .set<Position>({player.get<Position>()->x, player.get<Position>()->y});
            std::cout << "New projectile created" << std::endl;
        }
    }
    
    return 0;
} 