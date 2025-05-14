# RLEngine

RLEngine is a game engine built using SDL2 and OpenGL ES. It provides a robust framework for developing 2D (and maybe some day 3D) games with support for various assets like fonts, images, scripts, sounds, and tilemaps.

## Table of Contents

- [Overview](#overview)
- [Installation](#installation)
- [Building](#building)
- [Running](#running)
- [ECS Framework](#ecs-framework)
- [ECS Migration](#ecs-migration)
- [Contributing](#contributing)
- [License](#license)

## Overview

RLEngine is designed to be a flexible and powerful game engine. It leverages SDL2 for window management and input handling, and OpenGL ES for rendering (with Vulkan support in progress). The engine supports various asset types and includes examples to help you get started.

## Installation

To get started with RLEngine, clone the repository:

```sh
git clone https://github.com/yourusername/RLEngine.git
cd RLEngine
```

## Building
### Prerequisites
 - CMake
 - A C++ compiler (GCC, Clang, or MSVC)
 - SDL2 development libraries

### Build Instructions
Configure the project using CMake:
```
cmake -B build -S .
```

Build the project:
```
cmake --build build --config Release
```

## Running

After building the project, you can run the engine executable:
```
./bin/RLEngine.exe
```

## ECS Framework

RLEngine now uses the Flecs Entity Component System (ECS) framework. Flecs is a fast and lightweight ECS that provides features such as:

- Archetype-based component storage for better performance
- Relationship-based components for expressing complex entity relationships
- Prefabs for entity templates and composition
- Native support for hierarchies

### Migration from Custom ECS

We're migrating from our custom ECS implementation to Flecs. The migration process is designed to be gradual, allowing developers to transition their code over time. A `FlecsBridge` class has been provided to maintain compatibility with the existing API.

For more information on migrating to Flecs, see the [Flecs Migration Guide](doc/FlecsMigrationGuide.md).

### Example Usage

Check out the [FlecsDirect.cpp](examples/FlecsDirect.cpp) example to see how to use Flecs directly in your game code.

## ECS Migration

We've successfully migrated from our custom Entity Component System implementation to [Flecs](https://github.com/SanderMertens/flecs), a fast and feature-rich ECS library. This migration provides:

- Better performance with archetype-based storage
- More efficient component queries
- Advanced features like relationships, prefabs, and reflection
- Lower maintenance overhead

### Migration Approach

We've implemented a compatibility layer called `FlecsBridge` that maintains a similar API to our original ECS while using Flecs under the hood. This allows for a gradual transition:

1. Systems can be migrated one at a time
2. Existing code continues to function through the bridge
3. New systems can use Flecs directly for maximum performance

### Known Issues

- When using `std::string` with Flecs, you may see warnings about "missing EcsType" - these can be safely ignored when using the bridge approach
- Some complex C++ types may require special handling for full reflection support
- The Flecs meta module integration is still in progress

### Documentation

For full details on the migration process, see [FlecsMigrationGuide.md](doc/FlecsMigrationGuide.md)

### Direct Flecs Usage

For new code, we recommend using Flecs directly rather than through the bridge:

```cpp
// Register components
world.component<TransformComponent>();
world.component<RigidBodyComponent>();

// Create a system
world.system<TransformComponent, RigidBodyComponent>()
    .each([](TransformComponent& t, RigidBodyComponent& rb) {
        t.Position.x += rb.Velocity.x * world.delta_time();
        t.Position.y += rb.Velocity.y * world.delta_time();
    });
```

## Contributing
Contributions are welcome! Please fork the repository and submit pull requests. For major changes, please open an issue first to discuss what you would like to change.

## License
This project is licensed under the GNU GPL v3.0 License. See the LICENSE file for details.