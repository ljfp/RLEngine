#pragma once

#include <glm/glm.hpp>

struct BoxColliderComponent
{
	uint16_t Width;
	uint16_t Height;
	glm::vec2 Offset;

	BoxColliderComponent(uint16_t Width = 0, uint16_t Height = 0, glm::vec2 Offset = { 0, 0 })
	{
		this->Width = Width;
		this->Height = Height;
		this->Offset = Offset;
	}
};