#include "Game.hpp"
#include "LevelLoader.hpp"

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
	: Window(nullptr), Renderer(nullptr), Camera{ 0.0f, 0.0f, 0.0f, 0.0f }, IsRunning(false), IsDebug(false)
{
	GameAssetManager = std::make_unique<AssetManager>();
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
	auto& Input = GameWorld.get_mut<InputState>();
	Input.Clear();

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
			Input.QuitRequested = true;
			break;
		case SDL_EVENT_KEY_DOWN:
			Input.PressedKeys.push_back(Event.key.key);
			if (Event.key.key == SDLK_ESCAPE)
			{
				Input.QuitRequested = true;
			}
			if (Event.key.key == SDLK_D)
			{
				Input.ToggleDebugRequested = true;
			}
			break;
		}
	}
}

void Game::Setup()
{
	LuaState.open_libraries(sol::lib::base, sol::lib::math, sol::lib::os);

	RegisterFlecsGameWorld(GameWorld);
	GameWorld.set<GameContext>(GameContext{ Renderer, GameAssetManager.get(), &Camera, &IsDebug, &IsRunning });
	GameWorld.set<InputState>(InputState{});
	GameWorld.set<MapBounds>(MapBounds{});
	GameWorld.set<CollisionState>(CollisionState{});

	RegisterScriptBindings(GameWorld, LuaState);
	RegisterFlecsSystems(GameWorld);

	LevelLoader Loader;
	Loader.LoadLevel(LuaState, GameWorld, GameAssetManager, Renderer, 2);
}

void Game::Update()
{
	// If we are too fast, wait until the next frame (skip when VSync handles pacing)
	if (CAP_FRAMES && !VSYNC)
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

	// This line moves the game forward (one tick) and runs all the systems.
	const bool WorldShouldContinue = GameWorld.progress(static_cast<float>(DeltaTime));
	IsRunning = IsRunning && WorldShouldContinue;
}

void Game::Run()
{
	Setup();
	while (IsRunning)
	{
		ProcessInput();
		Update();
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