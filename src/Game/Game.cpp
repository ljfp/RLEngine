#include "Game.hpp"
#include "../ECS/ECS.hpp"
#include "../Components/AnimationComponent.hpp"
#include "../Components/BoxColliderComponent.hpp"
#include "../Components/CameraFollowComponent.hpp"
#include "../Components/KeyboardControlComponent.hpp"
#include "../Components/HealthComponent.hpp"
#include "../Components/ProjectileEmitterComponent.hpp"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/SpriteComponent.hpp"
#include "../Components/TextLabelComponent.hpp"
#include "../Components/TransformComponent.hpp"
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

#include <SDL2/SDL_image.h>
#include <spdlog/spdlog.h>
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
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
	// TODO: set this flag to true when the user plays the game.
	SDL_SetWindowFullscreen(Window, SDL_WINDOW_FULLSCREEN);

	// Initialize Dear ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplSDL2_InitForSDLRenderer(Window, Renderer);
	ImGui_ImplSDLRenderer2_Init(Renderer);

	GameRegistry = std::make_unique<Registry>();
	GameAssetManager = std::make_unique<AssetManager>();
	GameEventBus = std::make_unique<EventBus>();

	// Initialize the camera with the entire screen area
	Camera.x = 0;
	Camera.y = 0;
	Camera.w = WindowWidth;
	Camera.h = WindowHeight;

	spdlog::info("Game is running.");
	IsDebug = false;
	IsRunning = true;
}

Game::~Game()
{
	if (Renderer)
	{
		SDL_DestroyRenderer(Renderer);
	}

	if (Window)
	{
		SDL_DestroyWindow(Window);
	}

	ImGui_ImplSDLRenderer2_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();

	SDL_Quit();
	spdlog::info("Game is closing.");
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

void Game::LoadLevel(uint8_t LevelNumber)
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

	// Add assets to the asset store
	GameAssetManager->AddTexture(Renderer, "tank-image", "./assets/images/tank-panther-right.png");
	GameAssetManager->AddTexture(Renderer, "truck-image", "./assets/images/truck-ford-right.png");
	GameAssetManager->AddTexture(Renderer, "chopper-image", "./assets/images/chopper-spritesheet.png");
	GameAssetManager->AddTexture(Renderer, "radar-image", "./assets/images/radar.png");
	GameAssetManager->AddTexture(Renderer, "jungle-tilemap", "./assets/tilemaps/jungle.png");
	GameAssetManager->AddTexture(Renderer, "bullet-image", "./assets/images/bullet.png");
	GameAssetManager->AddFont("charriot-font-20", "./assets/fonts/charriot.ttf", 20);
	GameAssetManager->AddFont("pico8-font-5", "./assets/fonts/pico8.ttf", 5);
	GameAssetManager->AddFont("pico8-font-10", "./assets/fonts/pico8.ttf", 10);

	// Load the tilemap
	uint16_t TileSize = 32;
	double TileScale = 3.0;
	uint16_t TilemapColums = 25;
	uint16_t TilemapRows = 20;

	std::fstream TilemapFile;
	TilemapFile.open("./assets/tilemaps/jungle.map", std::ios::in);

	for (uint16_t y = 0; y < TilemapRows; y++)
	{
		for (uint16_t x = 0; x < TilemapColums; x++)
		{
			char ch;
			// Read the first digit
			TilemapFile.get(ch);
			uint16_t SourceRectangleY = std::atoi(&ch) * TileSize;
			// Read the second digit
			TilemapFile.get(ch);
			uint16_t SourceRectangleX = std::atoi(&ch) * TileSize;
			// Skip the comma
			TilemapFile.ignore();

			Entity Tile = GameRegistry->CreateEntity();
			Tile.Group("Tiles");
			Tile.AddComponent<TransformComponent>(glm::vec2(x * (TileScale * TileSize), y * (TileScale * TileSize)), glm::vec2(TileScale, TileScale), 0.0);
			Tile.AddComponent<SpriteComponent>("jungle-tilemap", 0, TileSize, TileSize, false, SourceRectangleX, SourceRectangleY);
		}
	}

	TilemapFile.close();
	MapWidth = TilemapColums * TileSize * TileScale;
	MapHeight = TilemapRows * TileSize * TileScale;

	// Add entities to the game
	Entity Chopper = GameRegistry->CreateEntity();
	Chopper.Tag("Player");
	Chopper.AddComponent<TransformComponent>(glm::vec2(30.0, 300.0), glm::vec2(2.0, 2.0), 0.0);
	Chopper.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	Chopper.AddComponent<SpriteComponent>("chopper-image", 1, 32, 32);
	Chopper.AddComponent<AnimationComponent>(2, 10, true);
	Chopper.AddComponent<BoxColliderComponent>(32, 32);
	Chopper.AddComponent<ProjectileEmitterComponent>(glm::vec2(150.0, 150.0), 0, 10000, 10, true);
	Chopper.AddComponent<KeyboardControlComponent>(glm::vec2(0, -200), glm::vec2(0, 200), glm::vec2(-200, 0), glm::vec2(200, 0));
	Chopper.AddComponent<CameraFollowComponent>();
	Chopper.AddComponent<HealthComponent>(100);

	Entity Radar = GameRegistry->CreateEntity();
	Radar.AddComponent<TransformComponent>(glm::vec2(WindowWidth - 192, 10.0), glm::vec2(2.0, 2.0), 0.0);
	Radar.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	Radar.AddComponent<SpriteComponent>("radar-image", 2, 64, 64, true);
	Radar.AddComponent<AnimationComponent>(8, 5, true);

	Entity Tank = GameRegistry->CreateEntity();
	Tank.Group("Enemies");
	Tank.AddComponent<TransformComponent>(glm::vec2(640.0, 576.0), glm::vec2(2.0, 2.0), 0.0);
	Tank.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	Tank.AddComponent<SpriteComponent>("tank-image", 1, 32, 32);
	Tank.AddComponent<BoxColliderComponent>(32, 32);
	Tank.AddComponent<ProjectileEmitterComponent>(glm::vec2(300.0, 0.0), 5000, 2500, 10, false);
	Tank.AddComponent<HealthComponent>(100);

	Entity Truck = GameRegistry->CreateEntity();
	Truck.Group("Enemies");
	Truck.AddComponent<TransformComponent>(glm::vec2(160.0, 768.0), glm::vec2(2.0, 2.0), 0.0);
	Truck.AddComponent<RigidBodyComponent>(glm::vec2(0.0, 0.0));
	Truck.AddComponent<SpriteComponent>("truck-image", 1, 32, 32);
	Truck.AddComponent<BoxColliderComponent>(32, 32);
	Truck.AddComponent<ProjectileEmitterComponent>(glm::vec2(0.0, 300), 2000, 5000, 10, false);
	Truck.AddComponent<HealthComponent>(100);

	Entity Label = GameRegistry->CreateEntity();
	Label.AddComponent<TextLabelComponent>(glm::vec2(100.0, 100.0), "Hello World", "charriot-font-20", SDL_Color{ 255, 255, 255 }, true);
}

void Game::Setup()
{
	LoadLevel(1);
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
	GameRegistry->GetSystem<DamageSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<KeyboardControlSystem>().SubscribeToEvents(GameEventBus);
	GameRegistry->GetSystem<ProjectileEmitterSystem>().SubscribeToEvents(GameEventBus);

	// Invoke all systems that need to update
	GameRegistry->GetSystem<MovementSystem>().Update(DeltaTime);
	GameRegistry->GetSystem<ProjectileEmitterSystem>().Update(GameRegistry);
	GameRegistry->GetSystem<AnimationSystem>().Update();
	GameRegistry->GetSystem<CollisionSystem>().Update(GameEventBus);
	GameRegistry->GetSystem<CameraFollowSystem>().Update(Camera);
	GameRegistry->GetSystem<ProjectileLifecycleSystem>().Update();

	// Update all entities at the end of our frame
	GameRegistry->Update();
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