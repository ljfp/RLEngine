#pragma once

#include "../AssetManager/AssetManager.hpp"
#include <flecs.h>

#include <cstdint>
#include <memory>
#include <SDL3/SDL.h>
#include <sol/sol.hpp>

class LevelLoader
{
public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(sol::state& LuaState, flecs::world& World, const std::unique_ptr<AssetManager>& AssetManager, SDL_Renderer* Renderer, uint8_t LevelNumber);
private:
};