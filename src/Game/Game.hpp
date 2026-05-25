#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../ECS/FlecsGameWorld.hpp"
#include <SDL3/SDL.h>
#include <flecs.h>
#include <sol/sol.hpp>

#include <memory>

constexpr uint16_t FPS = 60;
constexpr uint16_t MILISECONDS_PER_FRAME = 1000 / 60;
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
	void Destroy();

	static uint16_t WindowWidth;
	static uint16_t WindowHeight;
	static uint16_t MapWidth;
	static uint16_t MapHeight;

private:
	SDL_Window *Window;
	SDL_Renderer *Renderer;
	SDL_FRect Camera;
	bool IsRunning;
	bool IsDebug;
	uint64_t MillisecondsPreviousFrame = 0;

	sol::state LuaState;

	std::unique_ptr<AssetManager> GameAssetManager;
	flecs::world GameWorld;
};
