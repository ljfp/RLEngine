#pragma once

#include "../ECS/ECS.hpp"
#include "../EventBus/Event.hpp"
#include <SDL2/SDL.h>

class KeyPressedEvent : public Event
{
public:
	SDL_Keycode KeyCode;
	KeyPressedEvent(SDL_Keycode KeyCode) : KeyCode(KeyCode) {}
};