#include "FlecsSystems.hpp"

static void CleanupDestroyedEntitiesSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	const auto PendingDestroy = World.lookup("PendingDestroy");
	if (PendingDestroy.id() != 0)
	{
		ecs_delete_with(World.c_ptr(), PendingDestroy.id());
	}
}

void RegisterCleanupSystems(flecs::world& World)
{
	const auto Phase = World.lookup(CleanupPhaseName);
	World.system("CleanupDestroyedEntitiesSystem")
		.kind(Phase.id())
		.each(CleanupDestroyedEntitiesSystemTask);
}