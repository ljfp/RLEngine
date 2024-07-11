#pragma once

#include <glm/glm.hpp>

struct TransformComponent
{
	glm::vec2 Position;
	glm::vec2 Scale;
	double Rotation;

	TransformComponent
	(
		glm::vec2 Position = glm::vec2(0, 0),
		glm::vec2 Scale = glm::vec2(1, 1),
		double Rotation = 0.0
	)
	{
		this->Position = Position;
		this->Scale = Scale;
		this->Rotation = Rotation;
	}
};