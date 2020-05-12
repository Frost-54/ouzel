// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#include "GameSample.hpp"
#include "MainMenu.hpp"

using namespace ouzel;
using namespace input;

GameSample::GameSample()
{
    handler.keyboardHandler = [](const KeyboardEvent& event) {
        if (event.type == Event::Type::keyboardKeyPress)
        {
            switch (event.key)
            {
                case Keyboard::Key::Escape:
                case Keyboard::Key::Menu:
                case Keyboard::Key::Back:
                    engine->getSceneManager().setScene(std::make_unique<MainMenu>());
                    return true;
                default:
                    break;
            }
        }
        else if (event.type == Event::Type::keyboardKeyRelease)
        {
            switch (event.key)
            {
                case Keyboard::Key::Escape:
                case Keyboard::Key::Menu:
                case Keyboard::Key::Back:
                    return true;
                default:
                    break;
            }
        }

        return false;
    };

    handler.mouseHandler = [](const MouseEvent& event) {
        if (event.type == Event::Type::mousePress)
        {

        }
        return false;
    };

    handler.touchHandler = [](const TouchEvent& event) {
        if (event.type == Event::Type::touchBegin)
        {

        }
        return false;
    };

    handler.gamepadHandler = [](const GamepadEvent& event) {
        if (event.type == Event::Type::gamepadButtonChange)
        {
            if (event.pressed &&
                event.button == Gamepad::Button::faceRight)
                engine->getSceneManager().setScene(std::make_unique<MainMenu>());
        }

        return false;
    };

    handler.uiHandler = [](const UIEvent&) {
        return false;
    };

    camera.setClearColorBuffer(true);
    camera.setClearColor(ouzel::Color(64, 0, 0));
    
    engine->getEventDispatcher().addEventHandler(handler);

    addLayer(&layer);
    cameraActor.addComponent(&camera);
    layer.addChild(&cameraActor);
}
