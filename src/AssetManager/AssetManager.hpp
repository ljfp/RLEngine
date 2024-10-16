#pragma once

#include <map>
#include <SDL2/SDL.h>
#include <string>


class AssetManager
{
public:
	AssetManager();
	~AssetManager();

	void ClearAssets();
	void AddTexture(SDL_Renderer* Renderer, const std::string& AssetID, const std::string& FilePath);
	SDL_Texture* GetTexture(const std::string& AssetID) const;

private:
	std::map<std::string, SDL_Texture*> Textures;
	// TODO: Add support for fonts and sounds.
};