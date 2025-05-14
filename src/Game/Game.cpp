#include "Game.hpp"
#include "../AssetManager/AssetManager.hpp"
#include "../EventBus/EventBus.hpp"
#include "../Events/CollisionEvent.hpp"
#include "../Events/KeyPressedEvent.hpp"
#include "../Systems/MovementSystemFlecs.hpp"
#include "../Systems/RenderSystem.hpp"
#include "../Systems/RenderSystemFlecs.hpp"
#include "../Systems/AnimationSystem.hpp"
#include "../Systems/CollisionSystem.hpp"
#include "../Systems/RenderColliderSystem.hpp"
#include "../Systems/DamageSystem.hpp"
#include "../Systems/KeyboardControlSystemFlecs.hpp"
#include "../Systems/CameraFollowSystem.hpp"
#include "../Systems/ProjectileEmitterSystemFlecs.hpp"
#include "../Systems/ProjectileLifecycleSystem.hpp"
#include "../Systems/RenderTextSystem.hpp"
#include "../Systems/RenderHealthBarSystem.hpp"
#include "../Systems/ScriptSystem.hpp"
#include "../Systems/RenderDebugGUISystemFlecs.hpp"
#include "../Systems/DirectFlecsSystem.hpp"
#include "LevelLoader.hpp"
#include <SDL_ttf.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
#include <glm/gtc/type_ptr.hpp>
#include <flecs.h>
//#include <flecs/flecs/flecs.meta.h>

#include <SDL2/SDL_image.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <iostream>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_sdlrenderer2.h>
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
	GameRegistry = std::make_unique<FlecsBridge>();
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
		spdlog::warn("There's a problem getting display height (0 or negative value).");
	}

	// Create window with fixed resolution that matches our camera size
	Window = SDL_CreateWindow("RoguelikeEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1024, 768, SDL_WINDOW_VULKAN);
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
	Camera = { 0, 0, 1024, 768 };

	SDL_SetWindowFullscreen(Window, 0);
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

// Register custom types with Flecs for proper reflection
void RegisterCustomTypes(flecs::world& world) {
    spdlog::info("Registering custom types with Flecs");
    
    // Register glm::vec2
    world.component<glm::vec2>()
        .member<float>("x")
        .member<float>("y");
    
    // Register SDL_Rect
    world.component<SDL_Rect>()
        .member<int>("x")
        .member<int>("y")
        .member<int>("w")
        .member<int>("h");
    
    // Register SDL_RendererFlip as an enum
    world.component<SDL_RendererFlip>();
    
    // For std::string, we'll just register it and ignore the errors
    // Since we're implementing a bridge pattern, we don't need full
    // reflection support for all types
    world.component<std::string>();
}

// Register component types directly with Flecs
void RegisterComponents(flecs::world& world) {
    spdlog::info("Registering components with Flecs");
    
    // Register core components
    world.component<TransformComponent>()
        .member<glm::vec2>("Position")
        .member<glm::vec2>("Scale")
        .member<double>("Rotation");

    world.component<RigidBodyComponent>()
        .member<glm::vec2>("Velocity");
    
    world.component<SpriteComponent>()
        .member<std::string>("AssetID")
        .member<uint16_t>("Width")
        .member<uint16_t>("Height")
        .member<uint8_t>("ZIndex")
        .member<bool>("IsFixed")
        .member<SDL_RendererFlip>("Flip")
        .member<SDL_Rect>("SrcRect");
        
    // Register remaining components without the incorrect macro
    world.component<AnimationComponent>();
    world.component<BoxColliderComponent>();
    world.component<ProjectileComponent>();
    world.component<ProjectileEmitterComponent>();
    world.component<CameraFollowComponent>();
    world.component<KeyboardControlComponent>();
    world.component<HealthComponent>();
    world.component<TextLabelComponent>();
    world.component<ScriptComponent>();
}

void Game::Setup()
{
	// Register custom types and components with Flecs world
	RegisterCustomTypes(GameRegistry->GetWorld());
	RegisterComponents(GameRegistry->GetWorld());
	
	// Create core systems groups in Flecs
    GameRegistry->GetWorld().entity("Projectiles");

    GameRegistry->GetWorld().entity("Enemies");

    GameRegistry->GetWorld().entity("Obstacles");

    GameRegistry->GetWorld().entity("Player");
    
    // Add and initialize movement system
    MovementSystemFlecs& movementSystem = GameRegistry->AddSystem<MovementSystemFlecs>();
    movementSystem.SetWorld(&GameRegistry->GetWorld());
    
    // Add Flecs render system
    GameRegistry->AddSystem<RenderSystemFlecs>();
    // Set world reference for render system
    GameRegistry->GetSystem<RenderSystemFlecs>().SetWorld(&GameRegistry->GetWorld());
    
    // Add basic systems
	GameRegistry->AddSystem<AnimationSystem>();
	GameRegistry->AddSystem<CollisionSystem>();
	GameRegistry->AddSystem<RenderColliderSystem>();
	GameRegistry->AddSystem<DamageSystem>();
	
	// Add and initialize keyboard control system
	KeyboardControlSystemFlecs& keyboardSystem = GameRegistry->AddSystem<KeyboardControlSystemFlecs>();
	keyboardSystem.SetWorld(&GameRegistry->GetWorld());
	
	GameRegistry->AddSystem<CameraFollowSystem>();
	
	// Add and initialize projectile system
    ProjectileEmitterSystemFlecs& projectileSystem = GameRegistry->AddSystem<ProjectileEmitterSystemFlecs>();
    projectileSystem.SetWorld(&GameRegistry->GetWorld());
    
    // More basic systems
	GameRegistry->AddSystem<ProjectileLifecycleSystem>();
	GameRegistry->AddSystem<RenderTextSystem>();
	GameRegistry->AddSystem<RenderHealthBarSystem>();
	
	// Add debug GUI system
    GameRegistry->AddSystem<RenderDebugGUISystemFlecs>();

	// Add script system
    GameRegistry->AddSystem<ScriptSystem>();

	// Register systems directly with Flecs 
	DirectFlecsSystem::RegisterSystems(GameRegistry->GetWorld());

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
		uint16_t TimeToWait = MILISECONDS_PER_FRAME - (SDL_GetTicks64() - MillisecondsPreviousFrame);
		if (TimeToWait > 0 && TimeToWait <= MILISECONDS_PER_FRAME)
		{
			SDL_Delay(TimeToWait);
		}
	}

	double DeltaTime = (SDL_GetTicks64() - MillisecondsPreviousFrame) / 1000.0;
	MillisecondsPreviousFrame = SDL_GetTicks64();

	// Reset all event handlers for the current frame
	GameEventBus->Reset();

	// Perform the subscription of the events for all systems
	GameRegistry->GetSystem<DamageSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<KeyboardControlSystemFlecs>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<MovementSystemFlecs>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<ProjectileEmitterSystemFlecs>().SubscribeToEvents(GameEventBus);

	// Update the registry to process the entities that are waiting to be created/deleted
	GameRegistry->Update();

	// Invoke all systems that need to update
	GameRegistry->GetSystem<MovementSystemFlecs>().Update(DeltaTime);
	GameRegistry->GetSystem<ProjectileEmitterSystemFlecs>().Update(GameRegistry);
	GameRegistry->GetSystem<AnimationSystem>().Update();
	GameRegistry->GetSystem<CollisionSystem>().Update(GameEventBus);
	GameRegistry->GetSystem<CameraFollowSystem>().Update(Camera);
	GameRegistry->GetSystem<ProjectileLifecycleSystem>().Update();
	GameRegistry->GetSystem<ScriptSystem>().Update(DeltaTime, SDL_GetTicks64());
}

void Game::Render()
{
	SDL_SetRenderDrawColor(Renderer, 21, 21, 21, 255);
	SDL_RenderClear(Renderer);

	// Invoke all the systems that need to render
	GameRegistry->GetSystem<RenderSystemFlecs>().Update(Renderer, GameAssetManager, Camera, IsDebug);
	GameRegistry->GetSystem<RenderTextSystem>().Update(Renderer, GameAssetManager, Camera);
	GameRegistry->GetSystem<RenderHealthBarSystem>().Update(Renderer, GameAssetManager, Camera);
	if (IsDebug)
	{
		GameRegistry->GetSystem<RenderColliderSystem>().Update(Renderer, Camera);
	}
	
	// Always call Debug GUI system, but pass IsDebug flag to let it decide whether to render
	GameRegistry->GetSystem<RenderDebugGUISystemFlecs>().Update(Renderer, GameRegistry, Camera, IsDebug);

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