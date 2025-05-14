#pragma once

#include <flecs.h>
#include <string>
#include <memory>
#include <functional>
#include <typeindex>
#include <unordered_map>
#include <spdlog/spdlog.h>

// FlecsBridge provides a wrapper around Flecs that maintains a similar API
// to our custom ECS implementation, making migration easier
class FlecsBridge {
public:
    FlecsBridge() {
        world = std::make_unique<flecs::world>();
        spdlog::info("FlecsBridge created");
    }

    ~FlecsBridge() {
        spdlog::info("FlecsBridge destroyed");
    }

    // Entity wrapper to maintain compatibility
    class Entity {
    public:
        Entity(flecs::entity e) : entity(e) {}
        
        uint16_t GetID() const { 
            return static_cast<uint16_t>(entity.id()); 
        }
        
        void Kill() { 
            entity.destruct(); 
        }
        
        // Tag and group management using Flecs relationships
        void Tag(const std::string& tag) {
            // In Flecs, tags can be implemented as empty components or relationships
            auto tagEntity = entity.world().entity(tag.c_str());
            entity.add(tagEntity);
        }
        
        bool HasTag(const std::string& tag) const {
            auto tagEntity = entity.world().entity(tag.c_str());
            return entity.has(tagEntity);
        }
        
        void Group(const std::string& group) {
            // Using Flecs relationships for groups
            auto groupEntity = entity.world().entity(group.c_str());
            entity.add(flecs::ChildOf, groupEntity);
        }
        
        bool BelongsToGroup(const std::string& group) const {
            auto groupEntity = entity.world().entity(group.c_str());
            return entity.has(flecs::ChildOf, groupEntity);
        }
        
        // Component management
        template <typename T, typename ...TArgs>
        void AddComponent(TArgs&&... args) {
            entity.set<T>({std::forward<TArgs>(args)...});
        }
        
        template <typename T>
        void RemoveComponent() {
            entity.remove<T>();
        }
        
        template <typename T>
        bool HasComponent() const {
            return entity.has<T>();
        }
        
        template <typename T>
        T& GetComponent() const {
            return *entity.get_mut<T>();
        }
        
        flecs::entity GetFlecsEntity() const {
            return entity;
        }
        
        bool operator==(const Entity& other) const { 
            return entity == other.entity; 
        }
        
        bool operator!=(const Entity& other) const { 
            return entity != other.entity; 
        }
        
        bool operator<(const Entity& other) const { 
            return entity.id() < other.entity.id(); 
        }

    private:
        flecs::entity entity;
    };

    // Create an entity
    Entity CreateEntity() {
        auto e = world->entity();
        //spdlog::info("Entity created with ID: {}", e.id());
        return Entity(e);
    }

    // System management
    template <typename T, typename ...TArgs>
    T& AddSystem(TArgs&&... args) {
        // In Flecs, systems are created with a lambda or function
        // T is expected to be a class that defines the system behavior
        auto system = std::make_shared<T>(std::forward<TArgs>(args)...);
        systems[std::type_index(typeid(T))] = system;
        
        // For now, we'll just store the system but not actually hook it up to Flecs
        // This is a temporary solution until we properly migrate each system
        
        // The actual system logic will be executed by the Update method of each system
        // which is called manually in Game::Update
        return *(std::static_pointer_cast<T>(system));
    }
    
    template <typename T>
    void RemoveSystem() {
        auto it = systems.find(std::type_index(typeid(T)));
        if (it != systems.end()) {
            systems.erase(it);
        }
    }
    
    template <typename T>
    bool HasSystem() const {
        return systems.find(std::type_index(typeid(T))) != systems.end();
    }
    
    template <typename T>
    T& GetSystem() const {
        auto it = systems.find(std::type_index(typeid(T)));
        return *(std::static_pointer_cast<T>(it->second));
    }

    // Tag management
    Entity GetEntityByTag(const std::string& tag) const {
        auto tagEntity = world->entity(tag.c_str());
        
        // Use a query to find entities with this tag
        flecs::entity result = flecs::entity::null();
        
        world->each([&](flecs::entity e) {
            if (e.has(tagEntity)) {
                result = e;
                return false; // Stop iteration after finding first match
            }
            return true;
        });
        
        return result ? Entity(result) : Entity(flecs::entity::null());
    }
    
    // Group management
    std::vector<Entity> GetEntitiesByGroup(const std::string& group) const {
        std::vector<Entity> entities;
        auto groupEntity = world->entity(group.c_str());
        
        // Filter entities that are children of the group entity
        world->each([&](flecs::entity e) {
            if (e.has(flecs::ChildOf, groupEntity)) {
                entities.push_back(Entity(e));
            }
            return true;
        });
        
        return entities;
    }
    
    // Update the ECS world - called once per frame
    void Update() {
        world->progress();
    }
    
    // Get the underlying Flecs world
    flecs::world& GetWorld() {
        return *world;
    }

private:
    std::unique_ptr<flecs::world> world;
    std::unordered_map<std::type_index, std::shared_ptr<void>> systems;
};

// Base class for systems using Flecs
class FlecsBridgeSystem {
public:
    virtual ~FlecsBridgeSystem() = default;
    
    virtual void Run(flecs::iter& it) = 0;
    
    virtual void SetupQuery(flecs::world& world, flecs::system& system) = 0;
    
    // Set the world reference for the system to use
    virtual void SetWorld(flecs::world* world) {
        m_world = world;
    }
    
protected:
    flecs::world* m_world = nullptr;
};

// Template for creating systems with specific component requirements
template<typename... Components>
class FlecsBridgeSystemT : public FlecsBridgeSystem {
public:
    virtual void SetupQuery(flecs::world& world, flecs::system& system) override {
        // We'll implement this later when we're ready to use Flecs systems directly
        // For now, we're calling system Update methods manually
    }
    
    // Override this method in derived classes
    virtual void Run(flecs::iter& it) override = 0;
}; 