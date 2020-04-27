// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_GAMEPAD_HPP
#define OUZEL_INPUT_GAMEPAD_HPP

#include <cstdint>
#include <string>
#include "Controller.hpp"

namespace ouzel
{
    namespace input
    {
        class InputManager;

        class Gamepad final: public Controller
        {
            friend InputManager;
        public:
            enum class Axis
            {
                Unknown,
                LeftThumbX,
                LeftThumbY,
                RightThumbX,
                RightThumbY,
                LeftTrigger,
                RightTrigger
            };

            enum class Button
            {
                Unknown,
                DpadLeft,
                DpadRight,
                DpadUp,
                DpadDown,
                FaceBottom, // A on Xbox controller, cross on PS controller, B on Nintendo controller
                FaceRight, // B on Xbox controller, circle on PS controller, A on Nintendo controller
                FaceLeft, // X on Xbox controller, square on PS controller, Y on Nintendo controller
                FaceTop, // Y on Xbox controller, triangle on PS controller, X on Nintendo controller
                LeftShoulder, // L1 for Apple and PS controller
                LeftTrigger, // L2 for Apple and PS controller
                RightShoulder, // R1 for Apple and PS controller
                RightTrigger, // R2 for Apple and PS controller
                LeftThumb,
                RightThumb,
                Start,
                Back,
                Pause,
                LeftThumbLeft,
                LeftThumbRight,
                LeftThumbUp,
                LeftThumbDown,
                RightThumbLeft,
                RightThumbRight,
                RightThumbUp,
                RightThumbDown,
                Count
            };

            enum class Motor
            {
                All,
                Left,
                Right,
                Count
            };

            struct ButtonState final
            {
                bool pressed = false;
                float value = 0.0F;
            };

            Gamepad(InputManager& initInputManager, DeviceId initDeviceId);

            Gamepad(const Gamepad&) = delete;
            Gamepad& operator=(const Gamepad&) = delete;

            Gamepad(Gamepad&&) = delete;
            Gamepad& operator=(Gamepad&&) = delete;

            auto isAttached() const noexcept { return attached; }

            auto isAbsoluteDpadValues() const noexcept { return absoluteDpadValues; }
            void setAbsoluteDpadValues(bool newAbsoluteDpadValues);

            auto getPlayerIndex() const noexcept { return playerIndex; }
            void setPlayerIndex(std::int32_t newPlayerIndex);

            auto& getButtonState(Button button) const
            {
                return buttonStates[static_cast<std::uint32_t>(button)];
            }

            auto getVibration(Motor motor) { return vibration[static_cast<std::uint32_t>(motor)]; }
            void setVibration(Motor motor, float speed);

        protected:
            bool handleButtonValueChange(Gamepad::Button button, bool pressed, float value);

        private:
            ButtonState buttonStates[static_cast<std::uint32_t>(Button::Count)];
            std::int32_t playerIndex = -1;
            bool absoluteDpadValues = false;
            bool attached = false;
            float vibration[static_cast<std::uint32_t>(Motor::Count)]{0.0F};
        };
    } // namespace input
} // namespace ouzel

#endif // OUZEL_INPUT_GAMEPAD_HPP
