// Ouzel by Elviss Strazdins

#ifndef OUZEL_INPUT_INPUTSYSTEMTVOS_HPP
#define OUZEL_INPUT_INPUTSYSTEMTVOS_HPP

#include <memory>
#include <unordered_map>
#if defined(__OBJC__)
#  include <GameController/GameController.h>
typedef GCController* GCControllerPtr;
#else
#  include <objc/objc.h>
#  include <objc/NSObjCRuntime.h>
typedef id GCControllerPtr;
#endif

#include "../InputSystem.hpp"

namespace ouzel::input::tvos
{
    class GamepadDevice;

    class InputSystem final: public input::InputSystem
    {
    public:
        explicit InputSystem(const std::function<std::future<bool>(const Event&)>& initCallback);
        ~InputSystem() override;

        auto getKeyboardDevice() const noexcept { return keyboardDevice.get(); }

        void handleGamepadDiscoveryCompleted();
        void handleGamepadConnected(GCControllerPtr controller);
        void handleGamepadDisconnected(GCControllerPtr controller);

    private:
        void executeCommand(const Command& command) final;

        auto getNextDeviceId() noexcept
        {
            ++lastDeviceId.value;
            return lastDeviceId;
        }

        void startGamepadDiscovery();
        void stopGamepadDiscovery();

        void showVirtualKeyboard();
        void hideVirtualKeyboard();

        DeviceId lastDeviceId;
        std::unique_ptr<KeyboardDevice> keyboardDevice;
        std::unordered_map<GCControllerPtr, std::unique_ptr<GamepadDevice>> gamepadDevices;

        id connectDelegate = nil;
    };
}

#endif // OUZEL_INPUT_INPUTSYSTEMTVOS_HPP
