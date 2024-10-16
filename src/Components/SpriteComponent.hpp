#pragma once

#include <stdint.h>
#include <string>
#include <SDL2/SDL.h>

struct SpriteComponent
{
	std::string AssetID;
	uint8_t ZIndex; // Use layers instead of ZIndex.
	uint16_t Width;
	uint16_t Height;
	bool IsFixed;
	SDL_Rect SrcRect;

	SpriteComponent(std::string AssetID = "", uint8_t ZIndex = 0, uint16_t Width = 0, uint16_t Height = 0, bool IsFixed = false, uint16_t SrcRectX = 0, uint16_t SrcRectY = 0)
	{
		this->AssetID = AssetID;
		this->ZIndex = ZIndex;
		this->Width = Width;
		this->Height = Height;
		this-> IsFixed = IsFixed;
		this->SrcRect = { SrcRectX, SrcRectY, Width, Height };
	}
};