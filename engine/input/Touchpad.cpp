// Copyright 2015-2021 Elviss Strazdins. All rights reserved.

#include "Touchpad.hpp"
#include "../core/Engine.hpp"
#include "../events/EventDispatcher.hpp"

namespace ouzel::input
{
    Touchpad::Touchpad(InputManager& initInputManager, DeviceId initDeviceId, bool initScreen):
        Controller(initInputManager, Controller::Type::touchpad, initDeviceId),
        screen(initScreen)
    {
    }

    bool Touchpad::handleTouchBegin(std::uint64_t touchId, const Vector2F& position, float force)
    {
        auto event = std::make_unique<TouchEvent>();
        event->type = Event::Type::touchBegin;
        event->touchpad = this;
        event->touchId = touchId;
        event->position = position;
        event->force = force;

        touchPositions[touchId] = position;

        return engine->getEventDispatcher().dispatchEvent(std::move(event));
    }

    bool Touchpad::handleTouchEnd(std::uint64_t touchId, const Vector2F& position, float force)
    {
        auto event = std::make_unique<TouchEvent>();
        event->type = Event::Type::touchEnd;
        event->touchpad = this;
        event->touchId = touchId;
        event->position = position;
        event->force = force;

        const auto i = touchPositions.find(touchId);

        if (i != touchPositions.end())
            touchPositions.erase(i);

        return engine->getEventDispatcher().dispatchEvent(std::move(event));
    }

    bool Touchpad::handleTouchMove(std::uint64_t touchId, const Vector2F& position, float force)
    {
        auto event = std::make_unique<TouchEvent>();
        event->type = Event::Type::touchMove;
        event->touchpad = this;
        event->touchId = touchId;
        event->difference = position - touchPositions[touchId];
        event->position = position;
        event->force = force;

        touchPositions[touchId] = position;

        return engine->getEventDispatcher().dispatchEvent(std::move(event));
    }

    bool Touchpad::handleTouchCancel(std::uint64_t touchId, const Vector2F& position, float force)
    {
        auto event = std::make_unique<TouchEvent>();
        event->type = Event::Type::touchCancel;
        event->touchpad = this;
        event->touchId = touchId;
        event->position = position;
        event->force = force;

        const auto i = touchPositions.find(touchId);

        if (i != touchPositions.end())
            touchPositions.erase(i);

        return engine->getEventDispatcher().dispatchEvent(std::move(event));
    }
}
