# Migrating from Custom ECS to Flecs

This document provides a comprehensive guide on migrating your codebase from our custom Entity-Component-System (ECS) implementation to Flecs, a more robust and efficient external ECS library.

## Table of Contents

1. [Migration Strategy](#migration-strategy)
2. [FlecsBridge Approach](#flecsbridge-approach)
3. [Direct Flecs Usage](#direct-flecs-usage)
4. [Component Registration](#component-registration)
5. [System Migration](#system-migration)
6. [Entity Management](#entity-management)
7. [Tag and Group Migration](#tag-and-group-migration)
8. [Common Issues and Solutions](#common-issues-and-solutions)
9. [Performance Considerations](#performance-considerations)

## Migration Strategy

We've implemented a gradual migration strategy using a compatibility layer called `FlecsBridge` that mimics the API of our original ECS. This approach allows us to:

1. Make minimal changes to existing game code
2. Incrementally migrate systems over time
3. Test and validate each migrated component
4. Eventually transition to direct Flecs usage for maximum efficiency

## FlecsBridge Approach

### What is FlecsBridge?

`FlecsBridge` is a wrapper around Flecs that maintains a similar API to our custom ECS. This allows most of the codebase to continue functioning with minimal changes during the migration.

Key features:
- Entity wrapper with familiar methods
- Component management with a similar API
- Tag and group handling using Flecs relationships
- System storage and execution

### How to Use FlecsBridge

1. Replace `Registry` with `FlecsBridge`:
```cpp
// Before
std::unique_ptr<Registry> GameRegistry;

// After
std::unique_ptr<FlecsBridge> GameRegistry;
```

2. Use the bridge `Entity` class:
```cpp
// Before
Entity entity = Registry->CreateEntity();

// After
FlecsBridge::Entity entity = Registry->CreateEntity();
```

3. Update system implementations:
```cpp
// Before (custom ECS)
class MovementSystem : public System { ... };

// After (using bridge)
class MovementSystemFlecs : public FlecsBridgeSystemT<TransformComponent, RigidBodyComponent> { ... };
```

## Direct Flecs Usage

For new systems or when fully migrating a system, direct Flecs usage provides the best performance and access to Flecs features.

### Creating Systems Directly

```cpp
// Direct Flecs system
void CreateMovementSystem(flecs::world& world) {
    world.system<TransformComponent, RigidBodyComponent>()
        .each([](flecs::entity e, TransformComponent& transform, RigidBodyComponent& rigidBody) {
            // System logic here
            transform.Position.x += rigidBody.Velocity.x * e.delta_time();
            transform.Position.y += rigidBody.Velocity.y * e.delta_time();
        });
}
```

### Filters and Queries

Flecs provides powerful filtering capabilities:

```cpp
// Only process entities with Player tag
world.system<TransformComponent, RigidBodyComponent>()
    .term(world.entity("Player")) // Filter by tag
    .each([](TransformComponent& transform, RigidBodyComponent& rigidBody) {
        // Process player entities
    });
```

## Component Registration

Components should be registered with Flecs to enable reflection and optimized storage:

```cpp
void RegisterComponents(flecs::world& world) {
    world.component<TransformComponent>()
        .member<glm::vec2>("Position")
        .member<glm::vec2>("Scale")
        .member<double>("Rotation");

    world.component<RigidBodyComponent>()
        .member<glm::vec2>("Velocity");
    
    // Add more component registrations
}
```

## System Migration

When migrating a system, follow these steps:

1. Create a new class for your Flecs system version:
```cpp
class MovementSystemFlecs : public FlecsBridgeSystemT<TransformComponent, RigidBodyComponent> {
public:
    void Run(flecs::iter& it) override {
        auto transforms = it.field<TransformComponent>(1);
        auto rigidBodies = it.field<RigidBodyComponent>(2);
        
        for (auto i : it) {
            // Process components
            transforms[i].Position.x += rigidBodies[i].Velocity.x * it.delta_time();
            transforms[i].Position.y += rigidBodies[i].Velocity.y * it.delta_time();
            
            // Additional logic
        }
    }
};
```

2. Update the Game class to use your new system:
```cpp
// In Game::Setup()
GameRegistry->AddSystem<MovementSystemFlecs>();
```

3. Call the appropriate methods in the update loop:
```cpp
GameRegistry->GetSystem<MovementSystemFlecs>().Update(DeltaTime);
```

## Entity Management

### Creating Entities

```cpp
// Using FlecsBridge
FlecsBridge::Entity entity = Registry->CreateEntity();

// Direct Flecs usage
flecs::entity entity = world.entity();
```

### Adding Components

```cpp
// Using FlecsBridge
entity.AddComponent<TransformComponent>(glm::vec2(0, 0), glm::vec2(1, 1), 0.0);

// Direct Flecs usage
entity.set<TransformComponent>({glm::vec2(0, 0), glm::vec2(1, 1), 0.0});
```

### Removing Entities

```cpp
// Using FlecsBridge
entity.Kill();

// Direct Flecs usage
entity.destruct();
```

## Tag and Group Migration

Our custom ECS tags and groups map to Flecs relationships:

### Tags

```cpp
// Using FlecsBridge
entity.Tag("Player");
bool isPlayer = entity.HasTag("Player");

// Direct Flecs usage
entity.add(world.entity("Player"));
bool isPlayer = entity.has(world.entity("Player"));
```

### Groups

```cpp
// Using FlecsBridge
entity.Group("Enemies");
bool isEnemy = entity.BelongsToGroup("Enemies");

// Direct Flecs usage
entity.add(flecs::ChildOf, world.entity("Enemies"));
bool isEnemy = entity.has(flecs::ChildOf, world.entity("Enemies"));
```

## Common Issues and Solutions

### Empty Component Types

Flecs has a requirement that component types must have at least one data member. If you encounter crashes with assertion errors like this:

```
fatal: flecs.h: 20478: assert: _::type<T>::size() != 0 operation invalid for empty type (INVALID_PARAMETER)
```

This is because you're using a component type that has no data members. To fix this:

1. Add at least one data member to your empty component structs/classes:

```cpp
// Before - will cause Flecs to crash
struct CameraFollowComponent {
    CameraFollowComponent() = default;
};

// After - works with Flecs
struct CameraFollowComponent {
    bool Follow;
    
    CameraFollowComponent(bool Follow = true) {
        this->Follow = Follow;
    }
};
```

2. Update any code that creates these components to provide the required values

3. Make sure all components are registered with Flecs before they're used

### String Type Warnings

When using `std::string` with Flecs, you might see warnings like:

```
error: flecs.c: 51827: missing EcsType for type std.__cxx11.basic_string<char>'
```

These warnings occur because Flecs doesn't have built-in reflection support for complex C++ types like `std::string`. For our bridge approach, these warnings can be safely ignored as they don't prevent the application from running.

If you need full reflection support for strings in direct Flecs usage:

1. Use C-style strings (char arrays) instead of std::string
2. Create custom serialization/deserialization for std::string components
3. Use the Flecs meta module (requires additional setup)

### Query Performance

If query performance is poor:

1. Use cached queries for frequently executed queries
2. Filter queries appropriately to reduce the number of processed entities
3. Consider using Flecs terms to optimize filters

### Entity References

Entity references may behave differently in Flecs:

1. Don't store raw entity IDs across frames; use flecs::entity handles
2. Validate entity existence before accessing components
3. Use Flecs relationships for parent/child hierarchies

## Performance Considerations

Flecs offers significant performance improvements over our custom ECS:

1. Better cache coherence with archetype-based storage
2. More efficient queries for multi-component filtering
3. Optimized iteration over component data
4. Built-in multithreading support

To maximize performance:

1. Prefer direct Flecs usage over the bridge after migration
2. Use the Flecs Query Language (FQL) for complex queries
3. Register reflection data for components
4. Consider using Flecs' built-in prefab system instead of our custom prefabs

## Next Steps

1. Migrate one system at a time, starting with simpler systems
2. Test thoroughly after each migration
3. Gradually replace FlecsBridge usage with direct Flecs code
4. Update documentation and coding standards for new Flecs practices

By following this guide, you should be able to smoothly transition from our custom ECS to Flecs while maintaining functionality and improving performance. 