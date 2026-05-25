#include "FlecsSystems.hpp"

static void InputSystemTask(flecs::iter& Iter, size_t)
{
	auto World = Iter.world();
	auto& Input = World.get_mut<InputState>();
	auto& Context = World.get_mut<GameContext>();

	if (Input.QuitRequested && Context.IsRunning)
	{
		*Context.IsRunning = false;
	}

	if (Input.ToggleDebugRequested && Context.IsDebug)
	{
		*Context.IsDebug = !*Context.IsDebug;
	}
}

void RegisterInputSystems(flecs::world& World)
{
	const auto Phase = World.lookup(InputPhaseName);
	World.system("InputSystem")
		.kind(Phase.id())
		.each(InputSystemTask);
}