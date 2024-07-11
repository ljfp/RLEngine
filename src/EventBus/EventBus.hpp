#pragma once

#include <list>
#include <map>
#include <memory>
#include <typeindex>

#include "Event.hpp"
#include <spdlog/spdlog.h>

class IEventCallback
{
public:
	virtual ~IEventCallback() = default;
	void Execute(Event& AnEvent) { Call(AnEvent); }

private:
	virtual void Call(Event& AnEvent) = 0;
};


template <typename TOwner, typename TEvent>
class EventCallback : public IEventCallback
{
public:
	typedef void (TOwner::*CallbackFunction)(TEvent&);

	EventCallback(TOwner* AnOwnerInstance, CallbackFunction Callback)
	{
		this->OwnerInstance = AnOwnerInstance;
		this->Callback = Callback;
	}

	virtual ~EventCallback() override = default;
private:
	TOwner* OwnerInstance;
	CallbackFunction Callback;

	virtual void Call(Event& AnEvent) override
	{
		std::invoke(Callback, OwnerInstance, static_cast<TEvent&>(AnEvent));
	}
};

typedef std::list<std::unique_ptr<IEventCallback>> HandlerList;

class EventBus
{
public:
	EventBus()
	{
		spdlog::info("EventBus created");
	}

	~EventBus()
	{
		spdlog::info("EventBus destroyed");
	}

	// Clears the subscribers list
	void Reset()
	{
		Subscribers.clear();
	}

	// Subscribe to an event type <T>.
	// In our implementation, a listener subcribes to an event.
	// Example: EventBus->SubscribeToEvent<CollisionEvent>(this, &Game::OnCollision);
	template <typename TEvent, typename TOwner>
	void SubscribeToEvent(TOwner* OwnerInstance, void (TOwner::*CallbackFunction)(TEvent&))
	{
		if (Subscribers[typeid(TEvent)] == nullptr)
		{
			Subscribers[typeid(TEvent)] = std::make_unique<HandlerList>();
		}
		auto Subscriber = std::make_unique<EventCallback<TOwner, TEvent>>(OwnerInstance, CallbackFunction);
		Subscribers[typeid(TEvent)]->push_back(std::move(Subscriber));
	}

	// Emit an event type <T>.
	// In our implementation, as soon as something emits an event we execute all the listeners callback functions.
	// Example: EventBus->EmitEvent<CollisionEvent>(Player, Enemy);
	template <typename TEvent, typename ...TArgs>
	void EmitEvent(TArgs&& ...Args)
	{
		auto Handlers = Subscribers[typeid(TEvent)].get();
		if (Handlers)
		{
			for (auto Iterator = Handlers->begin(); Iterator != Handlers->end(); Iterator++)
			{
				auto Handler = Iterator->get();
				TEvent AnEvent(std::forward<TArgs>(Args)...);
				Handler->Execute(AnEvent);
			}
		}
	}

private:
	std::map<std::type_index, std::unique_ptr<HandlerList>> Subscribers;
};