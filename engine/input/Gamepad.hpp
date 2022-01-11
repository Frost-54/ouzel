// Ouzel by Elviss Strazdins

#ifndef OUZEL_INPUT_GAMEPAD_HPP
#define OUZEL_INPUT_GAMEPAD_HPP

#include <array>
#include <bitset>
#include <cstdint>
#include <string>
#include "Controller.hpp"

namespace ouzel::input
{
    class InputManager;

    class Gamepad final: public Controller
    {
        friend InputManager;
    public:
        enum class Axis
        {
            none,
            leftThumbX,
            leftThumbY,
            rightThumbX,
            rightThumbY,
            leftTrigger,
            rightTrigger
        };

        enum class Button
        {
            none,
            dPadLeft,
            dPadRight,
            dPadUp,
            dPadDown,
            faceBottom, // A on Xbox controller, cross on PS controller, B on Nintendo controller
            faceRight, // B on Xbox controller, circle on PS controller, A on Nintendo controller
            faceLeft, // X on Xbox controller, square on PS controller, Y on Nintendo controller
            faceTop, // Y on Xbox controller, triangle on PS controller, X on Nintendo controller
            leftShoulder, // L1 for Apple and PS controller
            leftTrigger, // L2 for Apple and PS controller
            rightShoulder, // R1 for Apple and PS controller
            rightTrigger, // R2 for Apple and PS controller
            leftThumb,
            rightThumb,
            start,
            back,
            pause,
            leftThumbLeft,
            leftThumbRight,
            leftThumbUp,
            leftThumbDown,
            rightThumbLeft,
            rightThumbRight,
            rightThumbUp,
            rightThumbDown,
            last = rightThumbDown
        };

        enum class Motor
        {
            all,
            left,
            right,
            last = right
        };

        Gamepad(InputManager& initInputManager, DeviceId initDeviceId);

        Gamepad(const Gamepad&) = delete;
        Gamepad& operator=(const Gamepad&) = delete;

        Gamepad(Gamepad&&) = delete;
        Gamepad& operator=(Gamepad&&) = delete;

        auto isAttached() const noexcept { return attached; }

        auto isAbsoluteDpadValues() const noexcept { return absoluteDpadValues; }
        void setAbsoluteDpadValues(bool newAbsoluteDpadValues);

        bool isRotationAllowed() const noexcept { return rotationAllowed; }
        void setRotationAllowed(bool rotationAllowed);

        auto getPlayerIndex() const noexcept { return playerIndex; }
        void setPlayerIndex(std::int32_t newPlayerIndex);

        auto getButtonState(Button button) const
        {
            return buttonStates.test(static_cast<std::size_t>(button));
        }

        auto getVibration(Motor motor) { return vibration[static_cast<std::size_t>(motor)]; }
        void setVibration(Motor motor, float speed);

    private:
        bool handleButtonValueChange(Gamepad::Button button, bool pressed, float value);

        std::bitset<static_cast<std::size_t>(Button::last) + 1U> buttonStates;
        std::array<float, static_cast<std::size_t>(Button::last) + 1U> buttonValues{};
        std::int32_t playerIndex = -1;
        bool absoluteDpadValues = false;
        bool rotationAllowed = false;
        bool attached = false;
        std::array<float, static_cast<std::size_t>(Motor::last) + 1U> vibration{};
    };
}

#endif // OUZEL_INPUT_GAMEPAD_HPP
