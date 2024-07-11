#include "ECS.hpp"
#include <spdlog/spdlog.h>

// TODO: implement all the functions from ECS.hpp

uint8_t BaseComponent::NextID = 0;

uint16_t Entity::GetID() const
{
	return ID;
}

void Entity::Kill()
{
	registry->KillEntity(*this);
}

void Entity::Tag(const std::string &Tag)
{
	registry->TagEntity(*this, Tag);
}

bool Entity::HasTag(const std::string &Tag) const
{
	return registry->EntityHasTag(*this, Tag);
}

void Entity::Group(const std::string &Group)
{
	registry->GroupEntity(*this, Group);
}

bool Entity::BelongsToGroup(const std::string &Group) const
{
	return registry->EntityBelongsToGroup(*this, Group);
}

void System::AddEntityToSystem(Entity AnEntity)
{
	Entities.push_back(AnEntity);
}

void System::RemoveEntityFromSystem(Entity AnEntity)
{
	Entities.erase(
		std::remove_if(
			Entities.begin(),
			Entities.end(),
			[&AnEntity](Entity OtherEntity)
			{
				return AnEntity == OtherEntity;
			}),
		Entities.end()
		);
}

const std::vector<Entity> &System::GetSystemEntities() const
{
	return Entities;
}

const Signature &System::GetComponentSignature() const
{
	return ComponentSignature;
}

Entity Registry::CreateEntity()
{
	uint16_t EntityID;
	if (FreeEntityIDs.empty())
	{
		EntityID = EntityCounter++;
		if (EntityID >= EntityComponentSignatures.size())
		{
			EntityComponentSignatures.resize(EntityID + 1);
		}
	}
	else
	{
		EntityID = FreeEntityIDs.front();
		FreeEntityIDs.pop_front();
	}

	Entity entity(EntityID);
	entity.registry = this;
	EntitiesToBeAdded.insert(entity);

	spdlog::info("Entity created with ID: {}", EntityID);

	return entity;
}

void Registry::Update()
{
	// Processing the entities that are waiting to be created to the active Systems.
	for (auto Entity : EntitiesToBeAdded)
	{
		AddEntityToSystems(Entity);
	}
	EntitiesToBeAdded.clear();

	// Process the entities that are waiting to be killed from the active Systems.
	for (auto Entity : EntitiesToBeKilled)
	{
		RemoveEntityFromSystems(Entity);
		EntityComponentSignatures[Entity.GetID()].reset();

		// Remove the entity from the component pools.
		for (auto Pool : ComponentPools)
		{
			if (Pool)
			{
				Pool->RemoveEntityFromPool(Entity.GetID());
			}
		}

		// Make the entity ID avaiable to be reused.
		FreeEntityIDs.push_back(Entity.GetID());

		// Remove any traces of that entity from the tag/group maps.
		RemoveEntityTag(Entity);
		RemoveEntityFromGroup(Entity);
	}
	EntitiesToBeKilled.clear();
}

void Registry::KillEntity(Entity entity)
{
	EntitiesToBeKilled.insert(entity);
}

void Registry::AddEntityToSystems(Entity Entity)
{
	const auto EntityID = Entity.GetID();

	// Match the entity with all systems that require the entity's components
	const auto& EntityComponentSignature = EntityComponentSignatures[EntityID];

	// Loop all the systems
	for (auto& System : Systems)
	{
		const auto& SystemComponentSignature = System.second->GetComponentSignature();

		// Check if the system requires the entity's components
		if ((EntityComponentSignature & SystemComponentSignature) == SystemComponentSignature)
		{
			System.second->AddEntityToSystem(Entity);
		}
	}
}

void Registry::RemoveEntityFromSystems(Entity Entity)
{
	for (auto system : Systems)
	{
		system.second->RemoveEntityFromSystem(Entity);
	}
}

void Registry::TagEntity(Entity AnEntity, const std::string &Tag)
{
	EntityPerTag.emplace(Tag, AnEntity);
	TagPerEntity.emplace(AnEntity.GetID(), Tag);
}

bool Registry::EntityHasTag(Entity AnEntity, const std::string &Tag) const
{
	if (TagPerEntity.find(AnEntity.GetID()) == TagPerEntity.end())
	{
		return false;
	}
	else
	{
		return EntityPerTag.find(Tag)->second == AnEntity;
	}
}

Entity Registry::GetEntityByTag(const std::string &Tag) const
{
	return EntityPerTag.at(Tag);
}

void Registry::RemoveEntityTag(Entity AnEntity)
{
	auto TaggedEntity = TagPerEntity.find(AnEntity.GetID());
	if (TaggedEntity != TagPerEntity.end())
	{
		auto Tag = TaggedEntity->second;
		EntityPerTag.erase(Tag);
		TagPerEntity.erase(TaggedEntity);
	}
}

void Registry::GroupEntity(Entity AnEntity, const std::string &Group)
{
	EntitiesPerGroup.emplace(Group, std::set<Entity>());
	EntitiesPerGroup.at(Group).emplace(AnEntity);
	GroupPerEntity.emplace(AnEntity.GetID(), Group);
}

bool Registry::EntityBelongsToGroup(Entity AnEntity, const std::string &Group) const
{
	if (EntitiesPerGroup.find(Group) == EntitiesPerGroup.end()) { return false; }

	auto GroupEntities = EntitiesPerGroup.at(Group);
	return GroupEntities.find(AnEntity.GetID()) != GroupEntities.end();
}

std::vector<Entity> Registry::GetEntitiesByGroup(const std::string &Group) const
{
	auto &SetOfEntities = EntitiesPerGroup.at(Group);
	return std::vector<Entity>(SetOfEntities.begin(), SetOfEntities.end());
}

void Registry::RemoveEntityFromGroup(Entity AnEntity)
{
	auto GroupedEntity = GroupPerEntity.find(AnEntity.GetID());
	if (GroupedEntity != GroupPerEntity.end())
	{
		auto Group = EntitiesPerGroup.find(GroupedEntity->second);
		if (Group != EntitiesPerGroup.end())
		{
			auto EntityInGroup = Group->second.find(AnEntity);
			if (EntityInGroup != Group->second.end())
			{
				Group->second.erase(EntityInGroup);
			}
		}
		GroupPerEntity.erase(GroupedEntity);
	}
}
