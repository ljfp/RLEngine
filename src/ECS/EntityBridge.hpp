#pragma once

#include "ECS.hpp"
#include "FlecsBridge.hpp"

namespace EntityBridge {
    // Convert from original Entity to FlecsBridge::Entity
    inline FlecsBridge::Entity ToFlecsBridgeEntity(const Entity& entity, flecs::world& world) {
        return FlecsBridge::Entity(world.entity(entity.GetID()));
    }
    
    // Convert from FlecsBridge::Entity to original Entity (approximate conversion - limited functionality)
    inline Entity ToEntity(const FlecsBridge::Entity& entity, Registry* registry) {
        Entity result(entity.GetID());
        result.registry = registry;
        return result;
    }
} 