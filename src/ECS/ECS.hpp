#pragma once

#include <stdint.h>
#include <bitset>
#include <deque>
#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <vector>

#include <spdlog/spdlog.h>


const uint8_t MAX_COMPONENTS = 64;
typedef std::bitset<MAX_COMPONENTS> Signature;

struct BaseComponent
{
protected:
	static uint8_t NextID;

};

// Use to assign a unique ID to a component type.
template <typename T>
class Component : public BaseComponent
{
public:
	static uint8_t GetID()
	{
		static auto ID = NextID++;
		return ID;
	}

};

class Registry;

class Entity
{
public:
	// x is the unique ID of the entity, which is just an integer number.
	Entity(uint16_t x) : ID(x){};
	uint16_t GetID() const;
	void Kill();

	//Entity& operator = (const Entity& Other) { ID = Other.ID; return *this;}
	Entity& operator = (const Entity& Other) = default;
	bool operator == (const Entity& Other) const { return ID == Other.ID;}
	bool operator != (const Entity& Other) const { return ID != Other.ID;}
	bool operator < (const Entity& Other) const { return ID < Other.ID;}
	bool operator > (const Entity& Other) const { return ID > Other.ID;}

	// Manage entity components
	template <typename T, typename ...TArgs> void AddComponent(TArgs&& ...args);
	template <typename T> void RemoveComponent();
	template <typename T> bool HasComponent() const;
	template <typename T> T& GetComponent() const;

	// Hold a pointer to the entity's owner registry.
	Registry* registry = nullptr;

	// Manage entity tags and groups.
	void Tag(const std::string& Tag);
	bool HasTag(const std::string& Tag) const;
	void Group(const std::string& Group);
	bool BelongsToGroup(const std::string& Group) const;

private:
	uint16_t ID;
};

// The system process components that have a specific signature.
class System
{
public:
	System() = default;
	virtual ~System() = default;

	void AddEntityToSystem(Entity AnEntity);
	void RemoveEntityFromSystem(Entity AnEntity);
	const std::vector<Entity> &GetSystemEntities() const;
	const Signature &GetComponentSignature() const;

	// Define component type T that entities must have to be considered by the system.
	template <typename TComponent>
	void RequireComponent();

private:
	// Which components an entity must have for the system to consider the entity.
	Signature ComponentSignature;
	std::vector<Entity> Entities;
};

class IPool
{
public:
	virtual ~IPool() = default;
	virtual void RemoveEntityFromPool(uint16_t EntityID) = 0;
};

// A Pool is just a vector (contiguous data) of objects of type T
template <typename T>
class Pool : public IPool
{
public:
	Pool(uint16_t Capacity = 100)
	{
		Size = 0;
		Data.resize(Capacity);
	}
	virtual ~Pool() = default;

	bool IsEmpty() const { return Size == 0; }

	uint16_t GetSize() const { return Size; }

	void Resize(uint16_t NewSize) {Data.resize(NewSize);}

	void Clear()
	{
		Data.clear();
		Size = 0;
	}

	void Add(T Object) { Data.push_back(Object); }

	void Set(uint16_t EntityID, T Object)
	{
		if (EntityIDToIndex.find(EntityID) != EntityIDToIndex.end())
		{
			uint16_t Index = EntityIDToIndex[EntityID];
			Data[Index] = Object;
		}
		else
		{
			uint16_t Index = Size;
			EntityIDToIndex.emplace(EntityID, Index);
			IndexToEntityID.emplace(Index, EntityID);
			if (Index >= Data.capacity())
			{
				Data.resize(Size * 2);
			}
			Data[Index] = Object;
			Size++;
		}
	}

	// When removing an entity, we swap the entity to be removed with the last entity in the pool to keep the array contiguous.
	void Remove(uint16_t EntityID)
	{
		// This way we avoid cache misses when iterating over the pool.
		uint16_t IndexToBeRemoved = EntityIDToIndex[EntityID];
		uint16_t IndexToBeSwapped = Size - 1; // We get the last element this way because the pool is 0-indexed.
		Data[IndexToBeRemoved] = Data[IndexToBeSwapped];

		// Update the EntityID and Indices maps to point to the correct elements.
		uint16_t EntityIDToBeSwapped = IndexToEntityID[IndexToBeSwapped];
		EntityIDToIndex[EntityIDToBeSwapped] = IndexToBeRemoved;
		IndexToEntityID[IndexToBeRemoved] = EntityIDToBeSwapped;

		EntityIDToIndex.erase(EntityID);
		IndexToEntityID.erase(IndexToBeSwapped);
		Size--;
	}

	void RemoveEntityFromPool(uint16_t EntityID) override
	{
		if (EntityIDToIndex.find(EntityID) != EntityIDToIndex.end())
		{
			Remove(EntityID);
		}
	}

	T& Get(uint16_t EntityID)
	{
		uint16_t Index = EntityIDToIndex[EntityID];
		return static_cast<T&>(Data[Index]);
	}

	T& operator [](uint16_t Index) { return Data[Index]; }

private:
	std::vector<T> Data;
	uint16_t Size;
	std::unordered_map<uint16_t, uint16_t> EntityIDToIndex;
	std::unordered_map<uint16_t, uint16_t> IndexToEntityID;
};

// The registry manages the creation and destruction of entities, systems, and components.
class Registry
{
public:
	Registry() { spdlog::info("Registry is created."); }

	~Registry() { spdlog::info("Registry is destroyed."); }

	// Entity management
	Entity CreateEntity();
	void Update();
	void KillEntity(Entity entity);
	void AddEntityToSystems(Entity Entity);
	void RemoveEntityFromSystems(Entity Entity);

	// Component management
	template <typename T, typename ...TArgs>
	void AddComponent(Entity entity, TArgs&& ...args);

	template <typename T>
	void RemoveComponent(Entity entity);

	template <typename T>
	bool HasComponent(Entity entity);

	template <typename T>
	T& GetComponent(Entity entity);

	// System management
	template <typename T, typename ...TArgs>
	void AddSystem(TArgs&& ...args);

	template <typename T>
	void RemoveSystem();

	template <typename T>
	bool HasSystem() const;

	template <typename T>
	T& GetSystem() const;

	// Tag management
	void TagEntity(Entity AnEntity, const std::string& Tag);
	bool EntityHasTag(Entity AnEntity, const std::string& Tag) const;
	Entity GetEntityByTag(const std::string& Tag) const;
	void RemoveEntityTag(Entity AnEntity);

	// Group management
	void GroupEntity(Entity AnEntity, const std::string& Group);
	bool EntityBelongsToGroup(Entity AnEntity, const std::string& Group) const;
	std::vector<Entity> GetEntitiesByGroup(const std::string& Group) const;
	void RemoveEntityFromGroup(Entity AnEntity);

private:
	// To keep track of how many entities were added to the scene.
	uint16_t EntityCounter = 0;
	std::set<Entity> EntitiesToBeAdded;
	std::set<Entity> EntitiesToBeKilled;

	// Vector of component pools. Each pool contains all the data for a certain component.
	std::vector<std::shared_ptr<IPool>> ComponentPools;

	// Vector of component signatures. The signature lets us know which components are turned "on" for an entity.
	std::vector<Signature> EntityComponentSignatures;

	// Unordered map of active systems.
	std::unordered_map<std::type_index, std::shared_ptr<System>> Systems;

	// List of free entity IDs to be reused.
	std::deque<uint16_t> FreeEntityIDs;

	// Entity tags (one tag name per entity).
	std::unordered_map<std::string, Entity> EntityPerTag;
	std::unordered_map<uint16_t, std::string> TagPerEntity;

	// Entity groups (a set of entities per group name)
	std::unordered_map<std::string, std::set<Entity>> EntitiesPerGroup;
	std::unordered_map<uint16_t, std::string> GroupPerEntity;
};

template <typename T>
inline void System::RequireComponent()
{
	const auto ComponentID = Component<T>::GetID();
	ComponentSignature.set(ComponentID);
}

template <typename T, typename ...TArgs>
void Registry::AddComponent(Entity entity, TArgs&& ...args)
{
	const auto ComponentID = Component<T>::GetID();
	const auto EntityID = entity.GetID();

	// If the component ID is greater than the current size of component pools, resize the vector.
	if (ComponentID >= ComponentPools.size())
	{
		ComponentPools.resize(ComponentID + 1, nullptr);
	}

	// If we don't have a pool for this component, create one.
	if (!ComponentPools[ComponentID])
	{
		std::shared_ptr<Pool<T>> NewComponentPool = std::make_shared<Pool<T>>();
		ComponentPools[ComponentID] = NewComponentPool;
	}

	// Get the component pool and add the component to the pool.
	std::shared_ptr<Pool<T>> ComponentPool = std::static_pointer_cast<Pool<T>>(ComponentPools[ComponentID]);

	// Create a New Component object of the type T and forward the arguments to the constructor.
	T NewComponent(std::forward<TArgs>(args)...);

	// Add the component to the pool.
	ComponentPool->Set(EntityID, NewComponent);

	// Update the entity component signature.
	EntityComponentSignatures[EntityID].set(ComponentID);

	spdlog::info("Component ID {} was added to Entity ID {}", ComponentID, EntityID);
}

template <typename T>
void Registry::RemoveComponent(Entity entity)
{
	const auto ComponentID = Component<T>::GetID();
	const auto EntityID = entity.GetID();

	// Remove the component from the pool associated with the entity.
	std::shared_ptr<Pool<T>> ComponentPool = std::static_pointer_cast<Pool<T>>(ComponentPools[ComponentID]);
	ComponentPool->Remove(EntityID);

	// Set this component signature to false for this entity.
	EntityComponentSignatures[EntityID].set(ComponentID, false);

	spdlog::info("Component ID {} was removed from Entity ID {}", ComponentID, EntityID);
}

template <typename T>
bool Registry::HasComponent(Entity entity)
{
	const auto ComponentID = Component<T>::GetID();
	const auto EntityID = entity.GetID();

	return EntityComponentSignatures[EntityID].test(ComponentID);
}

template <typename T>
T& Registry::GetComponent(Entity entity)
{
	const auto ComponentID = Component<T>::GetID();
	const auto EntityID = entity.GetID();

	std::shared_ptr<Pool<T>> ComponentPool = std::static_pointer_cast<Pool<T>>(ComponentPools[ComponentID]);
	return ComponentPool->Get(EntityID);
}

template <typename T, typename ...TArgs>
void Registry::AddSystem(TArgs&& ...args)
{
	std::shared_ptr<T> NewSystem = std::make_shared<T>(std::forward<TArgs>(args)...);
	Systems.insert(std::make_pair(std::type_index(typeid(T)), NewSystem));
}

template <typename T>
void Registry::RemoveSystem()
{
	auto system = Systems.find(std::type_index(typeid(T)));
	Systems.erase(system);
}

template <typename T>
bool Registry::HasSystem() const
{
	return Systems.find(std::type_index(typeid(T))) != Systems.end();
}

template <typename T>
T& Registry::GetSystem() const
{
	auto system = Systems.find(std::type_index(typeid(T)));
	return *(std::static_pointer_cast<T>(system->second));
}

template <typename T, typename ...TArgs>
void Entity::AddComponent(TArgs&& ...args)
{
	registry->AddComponent<T>(*this, std::forward<TArgs>(args)...);
}

template <typename T>
void Entity::RemoveComponent()
{
	registry->RemoveComponent<T>(*this);
}

template <typename T>
bool Entity::HasComponent() const
{
	return registry->HasComponent<T>(*this);
}

template <typename T>
T& Entity::GetComponent() const
{
	return registry->GetComponent<T>(*this);
}