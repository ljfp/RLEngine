#pragma once

#include <stdint.h>
#include <string>
#include <SDL2/SDL.h>

struct SpriteComponent
{
	std::string AssetID;
	uint16_t Width;
	uint16_t Height;
	uint8_t ZIndex; // Use layers instead of ZIndex.
	SDL_RendererFlip Flip = SDL_FLIP_NONE;
	bool IsFixed;
	SDL_Rect SrcRect;

	SpriteComponent(std::string AssetID = "", uint16_t Width = 0, uint16_t Height = 0, uint8_t ZIndex = 0, bool IsFixed = false, uint16_t SrcRectX = 0, uint16_t SrcRectY = 0)
	{
		this->AssetID = AssetID;
		this->Width = Width;
		this->Height = Height;
		this->ZIndex = ZIndex;
		this->Flip = SDL_FLIP_NONE; // Other options are: SDL_FLIP_HORIZONTAL, SDL_FLIP_VERTICAL
		this-> IsFixed = IsFixed;
		this->SrcRect = { SrcRectX, SrcRectY, Width, Height };
	}
};