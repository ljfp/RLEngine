#pragma once

#include <map>
#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <string>


class AssetManager
{
public:
	AssetManager();
	~AssetManager();

	void ClearAssets();

	void AddTexture(SDL_Renderer* Renderer, const std::string& AssetID, const std::string& FilePath);
	SDL_Texture* GetTexture(const std::string& AssetID) const;

	void AddFont(const std::string& AssetID, const std::string& FilePath, uint8_t FontSize);
	TTF_Font* GetFont(const std::string& AssetID);

private:
	std::map<std::string, SDL_Texture*> Textures;
	std::map<std::string, TTF_Font*> Fonts;
	// TODO: Add support for sounds.
};