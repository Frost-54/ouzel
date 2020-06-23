// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_GAMEPADDEVICE_HPP
#define OUZEL_INPUT_GAMEPADDEVICE_HPP

#include <future>
#include "InputDevice.hpp"
#include "Gamepad.hpp"

namespace ouzel::input
{
    class GamepadDevice: public InputDevice
    {
    public:
        GamepadDevice(InputSystem& initInputSystem, DeviceId initId);
        ~GamepadDevice() override;

        std::future<bool> handleButtonValueChange(Gamepad::Button button, bool pressed, float value);
    };
}

#endif // OUZEL_INPUT_GAMEPADDEVICE_HPP
