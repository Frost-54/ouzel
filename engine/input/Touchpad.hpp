// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_TOUCHPAD_HPP
#define OUZEL_INPUT_TOUCHPAD_HPP

#include <cstdint>
#include <unordered_map>
#include "Controller.hpp"
#include "../math/Vector.hpp"

namespace ouzel
{
    namespace input
    {
        class InputManager;

        class Touchpad final: public Controller
        {
            friend InputManager;
        public:
            Touchpad(InputManager& initInputManager, DeviceId initDeviceId, bool initScreen);

            auto isScreen() const noexcept { return screen; }

        protected:
            bool handleTouchBegin(std::uint64_t touchId, const Vector2F& position, float force = 1.0F);
            bool handleTouchEnd(std::uint64_t touchId, const Vector2F& position, float force = 1.0F);
            bool handleTouchMove(std::uint64_t touchId, const Vector2F& position, float force = 1.0F);
            bool handleTouchCancel(std::uint64_t touchId, const Vector2F& position, float force = 1.0F);

        private:
            std::unordered_map<std::uint64_t, Vector2F> touchPositions;
            bool screen = false;
        };
    } // namespace input
} // namespace ouzel

#endif // OUZEL_INPUT_TOUCHPAD_HPP
