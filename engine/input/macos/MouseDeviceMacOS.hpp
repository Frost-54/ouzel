// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_MOUSEDEVICEMACOS_HPP
#define OUZEL_INPUT_MOUSEDEVICEMACOS_HPP

#include "../MouseDevice.hpp"
#include "../Mouse.hpp"

namespace ouzel::input
{
    class CursorMacOS;

    class MouseDeviceMacOS final: public MouseDevice
    {
    public:
        MouseDeviceMacOS(InputSystem& initInputSystem,
                            DeviceId initId):
            MouseDevice(initInputSystem, initId)
        {
        }

        void setPosition(const Vector2F& position);
        auto isCursorVisible() const noexcept { return cursorVisible; }
        void setCursorVisible(bool newVisible);
        void setCursorLocked(bool locked);

        auto getCursor() const noexcept { return cursor; }
        void setCursor(CursorMacOS* newCursor);

    private:
        CursorMacOS* cursor = nullptr;
        bool cursorVisible = true;
    };
}

#endif // OUZEL_INPUT_MOUSEDEVICEMACOS_HPP
