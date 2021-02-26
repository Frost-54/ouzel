// Ouzel by Elviss Strazdins

#ifndef OUZEL_INPUT_MOUSEDEVICEMACOS_HPP
#define OUZEL_INPUT_MOUSEDEVICEMACOS_HPP

#include "../MouseDevice.hpp"
#include "../Mouse.hpp"

namespace ouzel::input::macos
{
    class InputSystem;
    class Cursor;

    class MouseDevice final: public input::MouseDevice
    {
    public:
        MouseDevice(InputSystem& initInputSystem, DeviceId initId);

        void setPosition(const Vector<float, 2>& position);
        auto isCursorVisible() const noexcept { return cursorVisible; }
        void setCursorVisible(bool newVisible);
        void setCursorLocked(bool locked);

        auto getCursor() const noexcept { return cursor; }
        void setCursor(Cursor* newCursor);

    private:
        Cursor* cursor = nullptr;
        bool cursorVisible = true;
    };
}

#endif // OUZEL_INPUT_MOUSEDEVICEMACOS_HPP
