// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_CONTROLLER_HPP
#define OUZEL_INPUT_CONTROLLER_HPP

#include "DeviceId.hpp"

namespace ouzel
{
    namespace input
    {
        class InputManager;

        class Controller
        {
        public:
            enum class Type
            {
                none,
                keyboard,
                mouse,
                touchpad,
                gamepad
            };

            Controller(InputManager& initInputManager, Type initType, DeviceId initDeviceId):
                inputManager(initInputManager), type(initType), deviceId(initDeviceId)
            {}
            virtual ~Controller() = default;

            auto getType() const noexcept { return type; }
            auto getDeviceId() const noexcept { return deviceId; }

        protected:
            InputManager& inputManager;
            Type type;
            DeviceId deviceId;
        };
    } // namespace input
} // namespace ouzel

#endif // OUZEL_INPUT_CONTROLLER_HPP
