#pragma once

#include <string>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>

struct TextLabelComponent
{
	glm::vec2 Position;
	std::string Text;
	std::string AssetID;
	SDL_Color Color;
	bool IsFixed;

	TextLabelComponent
	(
		glm::vec2 Position = glm::vec2(0),
		const std::string& Text = "",
		const std::string& AssetID = "",
		const SDL_Color& Color = { 0, 0, 0 },
		bool IsFixed = true
	)
	{
		this->Position = Position;
		this->Text = Text;
		this->AssetID = AssetID;
		this->Color = Color;
		this->IsFixed = IsFixed;
	}
};
