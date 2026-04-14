#pragma once

#include "../EventBus/Event.hpp"
#include <SDL3/SDL.h>

class KeyPressedEvent : public Event
{
public:
	SDL_Keycode KeyCode;
	KeyPressedEvent(SDL_Keycode KeyCode) : KeyCode(KeyCode) {}
};