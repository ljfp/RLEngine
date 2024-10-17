#include "Game.hpp"
#include "LevelLoader.hpp"
#include "../Systems/AnimationSystem.hpp"
#include "../Systems/CameraFollowSystem.hpp"
#include "../Systems/CollisionSystem.hpp"
#include "../Systems/DamageSystem.hpp"
#include "../Systems/KeyboardControlSystem.hpp"
#include "../Systems/MovementSystem.hpp"
#include "../Systems/RenderColliderSystem.hpp"
#include "../Systems/RenderDebugGUISystem.hpp"
#include "../Systems/RenderHealthBarSystem.hpp"
#include "../Systems/RenderTextSystem.hpp"
#include "../Systems/RenderSystem.hpp"
#include "../Systems/ProjectileEmitterSystem.hpp"
#include "../Systems/ProjectileLifecycleSystem.hpp"
#include "../Systems/ScriptSystem.hpp"

#include <SDL2/SDL_image.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <flecs.h>

uint16_t Game::WindowWidth;
uint16_t Game::WindowHeight;
uint16_t Game::MapWidth;
uint16_t Game::MapHeight;

Game::Game()
{
	IsRunning = false;
	IsDebug = false;
	GameRegistry = std::make_unique<flecs::world>();
	GameAssetManager = std::make_unique<AssetManager>();
	GameEventBus = std::make_unique<EventBus>();
	spdlog::info("Game is running.");
}

Game::~Game()
{
	spdlog::info("Game is closing.");
}

void Game::Initialize()
{
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		spdlog::error("Error initializing SDL.");
		return;
	}

	if (TTF_Init() != 0)
	{
		spdlog::error("Error initializing SDL_TTF.");
		return;
	}

	SDL_DisplayMode DisplayMode;
	SDL_GetCurrentDisplayMode(0, &DisplayMode);
	if (DisplayMode.w > 0)
	{
		WindowWidth = DisplayMode.w; // Implicit conversion from int to unsigned short
	}
	else
	{
		spdlog::warn("There's a problem getting display width (0 or negative value).");
	}

	if (DisplayMode.h > 0)
	{
		WindowHeight = DisplayMode.h; // Implicit conversion from int to unsigned short
	}
	else
	{
		spdlog::warn("There's a proble getting display height (0 or negative value).");
	}

	Window = SDL_CreateWindow("RoguelikeEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_BORDERLESS);
	if (!Window)
	{
		spdlog::error("Error creating SDL window.");
		return;
	}

	Renderer = SDL_CreateRenderer(Window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (!Renderer)
	{
		spdlog::error("Error creating SDL renderer.");
		return;
	}

	// Initialize Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(Window, Renderer);
	ImGui_ImplSDLRenderer2_Init(Renderer);

	// Initialize the camera with the entire screen area
	Camera = { 0, 0, WindowWidth, WindowHeight };

	SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN);
	IsRunning = true;
}

void Game::ProcessInput()
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		// Handle ImGui events
		ImGui_ImplSDL2_ProcessEvent(&Event);
		ImGuiIO& DebugIO = ImGui::GetIO();

		int MouseX, MouseY;
		const int MouseButtons = SDL_GetMouseState(&MouseX, &MouseY);

		DebugIO.MousePos = ImVec2(MouseX, MouseY);
		DebugIO.MouseDown[0] = MouseButtons & SDL_BUTTON(SDL_BUTTON_LEFT);
		DebugIO.MouseDown[1] = MouseButtons & SDL_BUTTON(SDL_BUTTON_RIGHT);

		// Handle core SDL events
		switch (Event.type)
		{
		case SDL_QUIT:
			IsRunning = false;
			break;
		case SDL_KEYDOWN:
			if (Event.key.keysym.sym == SDLK_ESCAPE)
			{
				IsRunning = false;
			}
			if (Event.key.keysym.sym == SDLK_d)
			{
				IsDebug = !IsDebug;
			}
			GameEventBus->EmitEvent<KeyPressedEvent>(Event.key.keysym.sym);
			break;
		}
	}
}

void Game::Setup()
{
	// Add the systems that need to be processed in our game
	GameRegistry->system<MovementSystem>();
	GameRegistry->system<RenderSystem>();
	GameRegistry->system<AnimationSystem>();
	GameRegistry->system<CollisionSystem>();
	GameRegistry->system<RenderColliderSystem>();
	GameRegistry->system<DamageSystem>();
	GameRegistry->system<KeyboardControlSystem>();
	GameRegistry->system<CameraFollowSystem>();
	GameRegistry->system<ProjectileEmitterSystem>();
	GameRegistry->system<ProjectileLifecycleSystem>();
	GameRegistry->system<RenderTextSystem>();
	GameRegistry->system<RenderHealthBarSystem>();
	GameRegistry->system<RenderDebugGUISystem>();
	GameRegistry->system<ScriptSystem>();

	// Create the bindings between C++ and Lua
	GameRegistry->get<ScriptSystem>().CreateLuaBindings(LuaState);

	LevelLoader Loader;
	LuaState.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
	Loader.LoadLevel(LuaState, GameRegistry, GameAssetManager, Renderer, 2);
}

void Game::Update()
{
	// If we are too fast, wait until the next frame
	if (CAP_FRAMES)
	{
		uint16_t TimeToWait = MILISECONDS_PER_FRAME - (SDL_GetTicks64() - MillisecondsPreviousFrame);
		if (TimeToWait > 0 && TimeToWait <= MILISECONDS_PER_FRAME)
		{
			SDL_Delay(TimeToWait);
		}
	}

	// TODO: replace double with std::float64_t when gcc has stable support for C++23
	double DeltaTime = (SDL_GetTicks64() - MillisecondsPreviousFrame) / 1000.0;

	MillisecondsPreviousFrame = SDL_GetTicks64();

	// Reset all event handlers for the current frame
	GameEventBus->Reset();

	// Perform the subscription of the events for all systems
	GameRegistry->get<DamageSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->get<KeyboardControlSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->get<MovementSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->get<ProjectileEmitterSystem>().SubscribeToEvents(GameEventBus);

	// Update the registry to process the entities that are waiting to be created/deleted
	GameRegistry->progress();

	// Invoke all systems that need to update
	GameRegistry->get<MovementSystem>().Update(DeltaTime);
	GameRegistry->get<ProjectileEmitterSystem>().Update(GameRegistry);
	GameRegistry->get<AnimationSystem>().Update();
	GameRegistry->get<CollisionSystem>().Update(GameEventBus);
	GameRegistry->get<CameraFollowSystem>().Update(Camera);
	GameRegistry->get<ProjectileLifecycleSystem>().Update();
	GameRegistry->get<ScriptSystem>().Update(DeltaTime, SDL_GetTicks64());

}

void Game::Render()
{
	SDL_SetRenderDrawColor(Renderer, 21, 21, 21, 255);
	SDL_RenderClear(Renderer);

	// Invoke all the systems that need to render
	GameRegistry->get<RenderSystem>().Update(Renderer, GameAssetManager, Camera);
	GameRegistry->get<RenderTextSystem>().Update(Renderer, GameAssetManager, Camera);
	GameRegistry->get<RenderHealthBarSystem>().Update(Renderer, GameAssetManager, Camera);
	if (IsDebug)
	{
		GameRegistry->get<RenderColliderSystem>().Update(Renderer, Camera);
		GameRegistry->get<RenderDebugGUISystem>().Update(Renderer, GameRegistry, Camera);
	}

	SDL_RenderPresent(Renderer);
}

void Game::Run()
{
	Setup();
	while (IsRunning)
	{
		ProcessInput();
		Update();
		Render();
	}
}

void Game::Destroy() {
	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	if (Renderer)
	{
		SDL_DestroyRenderer(Renderer);
	}

	if (Window)
	{
		SDL_DestroyWindow(Window);
	}

	SDL_Quit();
}
