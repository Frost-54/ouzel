// Ouzel by Elviss Strazdins

#ifdef __APPLE__
#  include <TargetConditionals.h>
#endif
#include <algorithm>
#include <stdexcept>
#include "InputManager.hpp"
#include "Gamepad.hpp"
#include "Keyboard.hpp"
#include "Mouse.hpp"
#include "Touchpad.hpp"
#include "../core/Engine.hpp"
#include "../events/EventDispatcher.hpp"
#include "../math/Scalar.hpp"

namespace ouzel::input
{
    void InputManager::update()
    {
        auto eventQueue = inputSystem.getEvents();

        while (!eventQueue.empty())
        {
            auto event = std::move(eventQueue.front());
            eventQueue.pop();

            event.first.set_value(handleEvent(event.second));
        }
    }

    bool InputManager::handleEvent(const InputSystem::Event& event)
    {
        switch (event.type)
        {
            case InputSystem::Event::Type::deviceConnect:
            {
                switch (event.deviceType)
                {
                    case Controller::Type::gamepad:
                    {
                        auto controller = std::make_unique<Gamepad>(*this, event.deviceId);
                        controllers.push_back(controller.get());

                        auto connectEvent = std::make_unique<GamepadEvent>();
                        connectEvent->type = Event::Type::gamepadConnect;
                        connectEvent->gamepad = controller.get();

                        controllerMap.insert(std::make_pair(event.deviceId, std::move(controller)));
                        return engine->getEventDispatcher().dispatchEvent(std::move(connectEvent));
                    }
                    case Controller::Type::keyboard:
                    {
                        auto controller = std::make_unique<Keyboard>(*this, event.deviceId);
                        controllers.push_back(controller.get());
                        if (!keyboard) keyboard = controller.get();

                        auto connectEvent = std::make_unique<KeyboardEvent>();
                        connectEvent->type = Event::Type::keyboardConnect;
                        connectEvent->keyboard = controller.get();

                        controllerMap.insert(std::make_pair(event.deviceId, std::move(controller)));
                        return engine->getEventDispatcher().dispatchEvent(std::move(connectEvent));
                    }
                    case Controller::Type::mouse:
                    {
                        auto controller = std::make_unique<Mouse>(*this, event.deviceId);
                        controllers.push_back(controller.get());
                        if (!mouse) mouse = controller.get();

                        auto connectEvent = std::make_unique<MouseEvent>();
                        connectEvent->type = Event::Type::mouseConnect;
                        connectEvent->mouse = controller.get();

                        controllerMap.insert(std::make_pair(event.deviceId, std::move(controller)));
                        return engine->getEventDispatcher().dispatchEvent(std::move(connectEvent));
                    }
                    case Controller::Type::touchpad:
                    {
                        auto controller = std::make_unique<Touchpad>(*this, event.deviceId, event.screen);
                        controllers.push_back(controller.get());
                        if (!touchpad) touchpad = controller.get();

                        auto connectEvent = std::make_unique<TouchEvent>();
                        connectEvent->type = Event::Type::touchpadConnect;
                        connectEvent->touchpad = controller.get();

                        controllerMap.insert(std::make_pair(event.deviceId, std::move(controller)));
                        return engine->getEventDispatcher().dispatchEvent(std::move(connectEvent));
                    }
                    default: throw std::runtime_error{"Invalid controller type"};
                }
                break;
            }
            case InputSystem::Event::Type::deviceDisconnect:
            {
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto controller = std::move(i->second);
                    controllerMap.erase(i);

                    if (const auto controllerIterator = std::find(controllers.begin(),
                                                                  controllers.end(),
                                                                  controller.get()); controllerIterator != controllers.end())
                        controllers.erase(controllerIterator);

                    switch (i->second->getType())
                    {
                        case Controller::Type::gamepad:
                        {
                            auto disconnectEvent = std::make_unique<GamepadEvent>();
                            disconnectEvent->type = Event::Type::gamepadDisconnect;
                            disconnectEvent->gamepad = static_cast<Gamepad*>(controller.get());
                            return engine->getEventDispatcher().dispatchEvent(std::move(disconnectEvent));
                        }
                        case Controller::Type::keyboard:
                        {
                            auto disconnectEvent = std::make_unique<KeyboardEvent>();
                            disconnectEvent->type = Event::Type::keyboardDisconnect;
                            disconnectEvent->keyboard = static_cast<Keyboard*>(controller.get());

                            if (controller.get() == keyboard)
                            {
                                keyboard = nullptr;
                                for (auto c : controllers)
                                    if (c->getType() == Controller::Type::keyboard)
                                        keyboard = static_cast<Keyboard*>(c);
                            }

                            return engine->getEventDispatcher().dispatchEvent(std::move(disconnectEvent));
                        }
                        case Controller::Type::mouse:
                        {
                            auto disconnectEvent = std::make_unique<MouseEvent>();
                            disconnectEvent->type = Event::Type::mouseDisconnect;
                            disconnectEvent->mouse = static_cast<Mouse*>(controller.get());

                            if (controller.get() == mouse)
                            {
                                mouse = nullptr;
                                for (auto c : controllers)
                                    if (c->getType() == Controller::Type::mouse)
                                        mouse = static_cast<Mouse*>(c);
                            }

                            return engine->getEventDispatcher().dispatchEvent(std::move(disconnectEvent));
                        }
                        case Controller::Type::touchpad:
                        {
                            auto disconnectEvent = std::make_unique<TouchEvent>();
                            disconnectEvent->type = Event::Type::touchpadDisconnect;
                            disconnectEvent->touchpad = static_cast<Touchpad*>(controller.get());

                            if (controller.get() == touchpad)
                            {
                                touchpad = nullptr;
                                for (auto c : controllers)
                                    if (c->getType() == Controller::Type::touchpad)
                                        touchpad = static_cast<Touchpad*>(c);
                            }

                            return engine->getEventDispatcher().dispatchEvent(std::move(disconnectEvent));
                        }
                        default: throw std::runtime_error{"Invalid controller type"};
                    }
                }
                break;
            }
            case InputSystem::Event::Type::deviceDiscoveryComplete:
                discovering = false;
                return true;
            case InputSystem::Event::Type::gamepadButtonChange:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto gamepad = static_cast<Gamepad*>(i->second.get());
                    return gamepad->handleButtonValueChange(event.gamepadButton, event.pressed, event.value);
                }
                break;
            case InputSystem::Event::Type::keyboardKeyPress:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto keyboardController = static_cast<Keyboard*>(i->second.get());
                    return keyboardController->handleKeyPress(event.keyboardKey);
                }
                break;
            case InputSystem::Event::Type::keyboardKeyRelease:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto keyboardController = static_cast<Keyboard*>(i->second.get());
                    return keyboardController->handleKeyRelease(event.keyboardKey);
                }
                break;
            case InputSystem::Event::Type::mousePress:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto mouseController = static_cast<Mouse*>(i->second.get());
                    return mouseController->handleButtonPress(event.mouseButton, event.position);
                }
                break;
            case InputSystem::Event::Type::mouseRelease:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto mouseController = static_cast<Mouse*>(i->second.get());
                    return mouseController->handleButtonRelease(event.mouseButton, event.position);
                }
                break;
            case InputSystem::Event::Type::mouseScroll:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto mouseController = static_cast<Mouse*>(i->second.get());
                    return mouseController->handleScroll(event.scroll, event.position);
                }
                break;
            case InputSystem::Event::Type::mouseMove:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto mouseController = static_cast<Mouse*>(i->second.get());
                    return mouseController->handleMove(event.position);
                }
                break;
            case InputSystem::Event::Type::mouseRelativeMove:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto mouseController = static_cast<Mouse*>(i->second.get());
                    return mouseController->handleRelativeMove(event.position);
                }
                break;
            case InputSystem::Event::Type::mouseLockChanged:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto mouseController = static_cast<Mouse*>(i->second.get());
                    return mouseController->handleCursorLockChange(event.locked);
                }
                break;
            case InputSystem::Event::Type::touchBegin:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto touchpadController = static_cast<Touchpad*>(i->second.get());
                    return touchpadController->handleTouchBegin(event.touchId, event.position);
                }
                break;
            case InputSystem::Event::Type::touchMove:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto touchpadController = static_cast<Touchpad*>(i->second.get());
                    return touchpadController->handleTouchMove(event.touchId, event.position);
                }
                break;
            case InputSystem::Event::Type::touchEnd:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto touchpadController = static_cast<Touchpad*>(i->second.get());
                    return touchpadController->handleTouchEnd(event.touchId, event.position);
                }
                break;
            case InputSystem::Event::Type::touchCancel:
                if (const auto i = controllerMap.find(event.deviceId); i != controllerMap.end())
                {
                    const auto touchpadController = static_cast<Touchpad*>(i->second.get());
                    return touchpadController->handleTouchCancel(event.touchId, event.position);
                }
                break;
            default:
                throw std::runtime_error{"Unhandled event"};
        }

        return false;
    }

    void InputManager::startDeviceDiscovery()
    {
        discovering = true;

        InputSystem::Command command{InputSystem::Command::Type::startDeviceDiscovery};
        inputSystem.addCommand(command);
    }

    void InputManager::stopDeviceDiscovery()
    {
        discovering = false;

        InputSystem::Command command{InputSystem::Command::Type::stopDeviceDiscovery};
        inputSystem.addCommand(command);
    }

    void InputManager::showVirtualKeyboard()
    {
        InputSystem::Command command{InputSystem::Command::Type::showVirtualKeyboard};
        inputSystem.addCommand(command);
    }

    void InputManager::hideVirtualKeyboard()
    {
        InputSystem::Command command{InputSystem::Command::Type::hideVirtualKeyboard};
        inputSystem.addCommand(command);
    }
}
