#include "AssetManager.hpp"

#include <SDL2/SDL_image.h>
#include <spdlog/spdlog.h>


AssetManager::AssetManager()
{
	spdlog::info("AssetManager created");
}

AssetManager::~AssetManager()
{
	ClearAssets();
	spdlog::info("AssetManager destroyed");
}

void AssetManager::ClearAssets()
{
	for (auto Texture : Textures)
	{
		SDL_DestroyTexture(Texture.second);
	}
	Textures.clear();
}

void AssetManager::AddTexture(SDL_Renderer* Renderer, const std::string &AssetID, const std::string &FilePath)
{
	SDL_Surface* Surface = IMG_Load(FilePath.c_str());
	SDL_Texture* Texture = SDL_CreateTextureFromSurface(Renderer, Surface);
	SDL_FreeSurface(Surface);

	// Add texture to the map
	Textures.emplace(AssetID, Texture);

	spdlog::info("Texture with AssetID: {} added", AssetID);
}

SDL_Texture *AssetManager::GetTexture(const std::string &AssetID) const
{
	if (Textures.find(AssetID) != Textures.end())
	{
		return Textures.at(AssetID);
	}
	else
	{
		spdlog::error("Texture with AssetID: {} not found", AssetID);
		return nullptr;
	}
}
