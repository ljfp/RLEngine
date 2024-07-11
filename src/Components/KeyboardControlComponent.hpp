#pragma once

#include <glm/glm.hpp>

struct KeyboardControlComponent
{
	glm::vec2 UpVelocity, DownVelocity, LeftVelocity, RightVelocity;
	KeyboardControlComponent(
		glm::vec2 UpVelocity = glm::vec2(0),
		glm::vec2 DownVelocity = glm::vec2(0),
		glm::vec2 LeftVelocity = glm::vec2(0),
		glm::vec2 RightVelocity = glm::vec2(0)
		)
	{
		this->UpVelocity = UpVelocity;
		this->DownVelocity = DownVelocity;
		this->LeftVelocity = LeftVelocity;
		this->RightVelocity = RightVelocity;
	}
};