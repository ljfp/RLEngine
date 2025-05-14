#pragma once

#include "../ECS/ECS.hpp"
#include "../EventBus/Event.hpp"

// Forward declaration to avoid circular dependency
class FlecsBridge;

class CollisionEvent : public Event
{
public:
	Entity A;
	Entity B;
	CollisionEvent(Entity A, Entity B) : A(A), B(B) {}
};