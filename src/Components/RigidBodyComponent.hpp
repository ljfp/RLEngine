#pragma once

#include <glm/glm.hpp>

struct RigidBodyComponent
{
	glm::vec2 Velocity;

	RigidBodyComponent(glm::vec2 Velocity = glm::vec2(0.0, 0.0))
	{
		this->Velocity = Velocity;
	}
};