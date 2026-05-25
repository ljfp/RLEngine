#include "FlecsSystems.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/TransformComponent.hpp"

#include <SDL3/SDL.h>
#include <glm/glm.hpp>

static void CameraFollowSystemTask(flecs::iter& Iter, size_t, const CameraFollowComponent&, const TransformComponent& Transform)
{
	auto World = Iter.world();
	auto& Context = World.get_mut<GameContext>();
	const auto& Bounds = World.get<MapBounds>();
	if (!Context.Camera)
	{
		return;
	}

	SDL_FRect& Camera = *Context.Camera;
	const float DesiredX = static_cast<float>(Transform.Position.x) - Camera.w / 2.0f;
	const float DesiredY = static_cast<float>(Transform.Position.y) - Camera.h / 2.0f;
	const float MaxX = static_cast<float>(Bounds.Width) - Camera.w;
	const float MaxY = static_cast<float>(Bounds.Height) - Camera.h;

	Camera.x = MaxX <= 0.0f ? MaxX / 2.0f : glm::clamp(DesiredX, 0.0f, MaxX);
	Camera.y = MaxY <= 0.0f ? MaxY / 2.0f : glm::clamp(DesiredY, 0.0f, MaxY);
}

void RegisterCameraSystems(flecs::world& World)
{
	const auto Phase = World.lookup(CameraPhaseName);
	World.system<const CameraFollowComponent, const TransformComponent>("CameraFollowSystem")
		.kind(Phase.id())
		.each(CameraFollowSystemTask);
}