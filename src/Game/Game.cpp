#include "Game.hpp"
#include "LevelLoader.hpp"
#include "../ECS/ECS.hpp"
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

#include <SDL3_image/SDL_image.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl3.h>
#include <imgui/imgui_impl_sdlrenderer3.h>
// TODO: remove this when gcc has stable support for C++23
//#include <stdfloat>

uint16_t Game::WindowWidth;
uint16_t Game::WindowHeight;
uint16_t Game::MapWidth;
uint16_t Game::MapHeight;

Game::Game()
{
	IsRunning = false;
	IsDebug = false;
	GameRegistry = std::make_unique<Registry>();
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
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
	{
		spdlog::error("Error initializing SDL.");
		return;
	}

	if (!TTF_Init())
	{
		spdlog::error("Error initializing SDL_TTF.");
		return;
	}

	const SDL_DisplayMode* DisplayMode = SDL_GetCurrentDisplayMode(SDL_GetPrimaryDisplay());
	if (DisplayMode && DisplayMode->w > 0)
	{
		WindowWidth = DisplayMode->w;
	}
	else
	{
		spdlog::warn("There's a problem getting display width (0 or negative value).");
	}

	if (DisplayMode && DisplayMode->h > 0)
	{
		WindowHeight = DisplayMode->h;
	}
	else
	{
		spdlog::warn("There's a proble getting display height (0 or negative value).");
	}

	Window = SDL_CreateWindow("RoguelikeEngine", WindowWidth, WindowHeight, SDL_WINDOW_BORDERLESS);
	if (!Window)
	{
		spdlog::error("Error creating SDL window.");
		return;
	}

	Renderer = SDL_CreateRenderer(Window, NULL);
	if (!Renderer)
	{
		spdlog::error("Error creating SDL renderer.");
		return;
	}

	if (VSYNC)
	{
		SDL_SetRenderVSync(Renderer, 1);
	}

	// Initialize Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL3_InitForSDLRenderer(Window, Renderer);
	ImGui_ImplSDLRenderer3_Init(Renderer);

	// Initialize the camera with the entire screen area
	Camera = { 0.0f, 0.0f, static_cast<float>(WindowWidth), static_cast<float>(WindowHeight) };

	SDL_SetWindowFullscreen(Window, true);
	IsRunning = true;
}

void Game::ProcessInput()
{
	SDL_Event Event;
	while (SDL_PollEvent(&Event))
	{
		// Handle ImGui events
		ImGui_ImplSDL3_ProcessEvent(&Event);
		ImGuiIO& DebugIO = ImGui::GetIO();

		float MouseX, MouseY;
		SDL_MouseButtonFlags MouseButtons = SDL_GetMouseState(&MouseX, &MouseY);

		DebugIO.MousePos = ImVec2(MouseX, MouseY);
		DebugIO.MouseDown[0] = MouseButtons & SDL_BUTTON_MASK(SDL_BUTTON_LEFT);
		DebugIO.MouseDown[1] = MouseButtons & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT);

		// Handle core SDL events
		switch (Event.type)
		{
		case SDL_EVENT_QUIT:
			IsRunning = false;
			break;
		case SDL_EVENT_KEY_DOWN:
			if (Event.key.key == SDLK_ESCAPE)
			{
				IsRunning = false;
			}
			if (Event.key.key == SDLK_D)
			{
				IsDebug = !IsDebug;
			}
			GameEventBus->EmitEvent<KeyPressedEvent>(Event.key.key);
			break;
		}
	}
}

void Game::Setup()
{
	// Add the systems that need to be processed in our game
	GameRegistry->AddSystem<MovementSystem>();
	GameRegistry->AddSystem<RenderSystem>();
	GameRegistry->AddSystem<AnimationSystem>();
	GameRegistry->AddSystem<CollisionSystem>();
	GameRegistry->AddSystem<RenderColliderSystem>();
	GameRegistry->AddSystem<DamageSystem>();
	GameRegistry->AddSystem<KeyboardControlSystem>();
	GameRegistry->AddSystem<CameraFollowSystem>();
	GameRegistry->AddSystem<ProjectileEmitterSystem>();
	GameRegistry->AddSystem<ProjectileLifecycleSystem>();
	GameRegistry->AddSystem<RenderTextSystem>();
	GameRegistry->AddSystem<RenderHealthBarSystem>();
	GameRegistry->AddSystem<RenderDebugGUISystem>();
	GameRegistry->AddSystem<ScriptSystem>();

	// Create the bindings between C++ and Lua
	GameRegistry->GetSystem<ScriptSystem>().CreateLuaBindings(LuaState);

	LevelLoader Loader;
	LuaState.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);
	Loader.LoadLevel(LuaState, GameRegistry, GameAssetManager, Renderer, 2);
}

void Game::Update()
{
	// If we are too fast, wait until the next frame
	if (CAP_FRAMES)
	{
		uint16_t TimeToWait = MILISECONDS_PER_FRAME - (SDL_GetTicks() - MillisecondsPreviousFrame);
		if (TimeToWait > 0 && TimeToWait <= MILISECONDS_PER_FRAME)
		{
			SDL_Delay(TimeToWait);
		}
	}

	// TODO: replace double with std::float64_t when gcc has stable support for C++23
	double DeltaTime = (SDL_GetTicks() - MillisecondsPreviousFrame) / 1000.0;

	MillisecondsPreviousFrame = SDL_GetTicks();

	// Reset all event handlers for the current frame
	GameEventBus->Reset();

	// Perform the subscription of the events for all systems
	GameRegistry->GetSystem<DamageSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<MovementSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<ProjectileEmitterSystem>().SubscribeToEvents(GameEventBus);

	// Update the registry to process the entities that are waiting to be created/deleted
	GameRegistry->Update();

	// Invoke all systems that need to update
	GameRegistry->GetSystem<MovementSystem>().Update(DeltaTime);
	GameRegistry->GetSystem<ProjectileEmitterSystem>().Update(GameRegistry);
	GameRegistry->GetSystem<AnimationSystem>().Update();
	GameRegistry->GetSystem<CollisionSystem>().Update(GameEventBus);
	GameRegistry->GetSystem<CameraFollowSystem>().Update(Camera);
	GameRegistry->GetSystem<ProjectileLifecycleSystem>().Update();
	GameRegistry->GetSystem<ScriptSystem>().Update(DeltaTime, SDL_GetTicks());

}

void Game::Render()
{
	SDL_SetRenderDrawColor(Renderer, 21, 21, 21, 255);
	SDL_RenderClear(Renderer);

	// Invoke all the systems that need to render
	GameRegistry->GetSystem<RenderSystem>().Update(Renderer, GameAssetManager, Camera);
	GameRegistry->GetSystem<RenderTextSystem>().Update(Renderer, GameAssetManager, Camera);
	GameRegistry->GetSystem<RenderHealthBarSystem>().Update(Renderer, GameAssetManager, Camera);
	if (IsDebug)
	{
		GameRegistry->GetSystem<RenderColliderSystem>().Update(Renderer, Camera);
		GameRegistry->GetSystem<RenderDebugGUISystem>().Update(Renderer, GameRegistry, Camera);
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
	ImGui_ImplSDLRenderer3_Shutdown();
	ImGui_ImplSDL3_Shutdown();
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