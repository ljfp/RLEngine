#pragma once

#include <stdint.h>

struct HealthComponent
{
	uint8_t HealthPercentage;

	HealthComponent(uint8_t HealthPercentage = 0)
	{
		this->HealthPercentage = HealthPercentage;
	}
};