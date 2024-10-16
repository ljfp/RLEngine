#pragma once

#include "../ECS/ECS.hpp"
#include "../AssetManager/AssetManager.hpp"
#include "../EventBus/EventBus.hpp"
#include <SDL2/SDL.h>
#include <sol/sol.hpp>

constexpr uint16_t FPS = 30;
constexpr uint16_t MILISECONDS_PER_FRAME = 1000 / 30;
constexpr bool VSYNC = true;
constexpr bool CAP_FRAMES = true;

class Game
{
public:
	Game();
	~Game();
	void Initialize();
	void Run();
	void Setup();
	void ProcessInput();
	void Update();
	void Render();
	void Destroy();

	static uint16_t WindowWidth;
	static uint16_t WindowHeight;
	static uint16_t MapWidth;
	static uint16_t MapHeight;

private:
	SDL_Window *Window;
	SDL_Renderer *Renderer;
	SDL_Rect Camera;
	bool IsRunning;
	bool IsDebug;
	uint64_t MillisecondsPreviousFrame = 0;

	sol::state LuaState;

	std::unique_ptr<Registry> GameRegistry;
	std::unique_ptr<AssetManager> GameAssetManager;
	std::unique_ptr<EventBus> GameEventBus;
};
