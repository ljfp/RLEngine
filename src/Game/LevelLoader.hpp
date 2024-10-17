#pragma once

#include "../AssetManager/AssetManager.hpp"
#include <cstdint>
#include <memory>
#include <SDL2/SDL.h>
#include <sol/sol.hpp>
#include <flecs.h>

class LevelLoader
{
public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(sol::state& LuaState, const std::unique_ptr<flecs::world>& Registry, const std::unique_ptr<AssetManager>& AssetManager, SDL_Renderer* Renderer, uint8_t LevelNumber);
private:
};
