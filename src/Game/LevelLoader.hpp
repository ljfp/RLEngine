#pragma once

#include "../AssetManager/AssetManager.hpp"
#include "../ECS/ECS.hpp"

#include <cstdint>
#include <memory>
#include <SDL2/SDL.h>
#include <sol/sol.hpp>

class LevelLoader
{
public:
	LevelLoader();
	~LevelLoader();

	void LoadLevel(sol::state& LuaState, const std::unique_ptr<Registry>& Registry, const std::unique_ptr<AssetManager>& AssetManager, SDL_Renderer* Renderer, uint8_t LevelNumber);
private:
};