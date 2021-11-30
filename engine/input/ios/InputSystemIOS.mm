// Ouzel by Elviss Strazdins

#include "InputSystemIOS.hpp"
#include "GamepadDeviceIOS.hpp"
#include "../../core/Engine.hpp"
#include "../../core/ios/NativeWindowIOS.hpp"
#include "../../utils/Log.hpp"

@interface ConnectDelegate: NSObject

@end

@implementation ConnectDelegate
{
    ouzel::input::ios::InputSystem* input;
}

- (id)initWithInput:(ouzel::input::ios::InputSystem*)initInput
{
    if (self = [super init])
        input = initInput;

    return self;
}

- (void)handleControllerConnected:(NSNotification*)notification
{
    input->handleGamepadConnected(notification.object);
}

- (void)handleControllerDisconnected:(NSNotification*)notification
{
    input->handleGamepadDisconnected(notification.object);
}
@end

namespace ouzel::input::ios
{
    InputSystem::InputSystem():
        keyboardDevice{std::make_unique<KeyboardDevice>(*this, getNextDeviceId())},
        touchpadDevice{std::make_unique<TouchpadDevice>(*this, getNextDeviceId(), true)}
    {
        connectDelegate = [[ConnectDelegate alloc] initWithInput:this];

        [[NSNotificationCenter defaultCenter] addObserver:connectDelegate
                                                 selector:@selector(handleControllerConnected:)
                                                     name:GCControllerDidConnectNotification
                                                   object:nil];

        [[NSNotificationCenter defaultCenter] addObserver:connectDelegate
                                                 selector:@selector(handleControllerDisconnected:)
                                                     name:GCControllerDidDisconnectNotification
                                                   object:nil];

        for (GCController* controller in [GCController controllers])
            handleGamepadConnected(controller);

        [GCController startWirelessControllerDiscoveryWithCompletionHandler:
            ^(void) { handleGamepadDiscoveryCompleted(); }];
    }

    InputSystem::~InputSystem()
    {
        if (connectDelegate)
        {
            [[NSNotificationCenter defaultCenter] removeObserver:connectDelegate];

            [connectDelegate release];
        }
    }

    void InputSystem::executeCommand(const Command& command)
    {
        switch (command.type)
        {
            case Command::Type::startDeviceDiscovery:
                startGamepadDiscovery();
                break;
            case Command::Type::stopDeviceDiscovery:
                stopGamepadDiscovery();
                break;
            case Command::Type::setAbsoluteDpadValues:
            {
                if (const auto inputDevice = getInputDevice(command.deviceId))
                {
                    const auto gamepadDevice = static_cast<GamepadDevice*>(inputDevice);
                    gamepadDevice->setAbsoluteDpadValues(command.absoluteDpadValues);
                }
                break;
            }
            case Command::Type::setRotationAllowed:
            {
                if (const auto inputDevice = getInputDevice(command.deviceId))
                {
                    const auto gamepadDevice = static_cast<GamepadDevice*>(inputDevice);
                    gamepadDevice->setRotationAllowed(command.rotationAllowed);
                }
                break;
            }
            case Command::Type::setPlayerIndex:
            {
                if (InputDevice* inputDevice = getInputDevice(command.deviceId))
                {
                    const auto gamepadDevice = static_cast<GamepadDevice*>(inputDevice);
                    gamepadDevice->setPlayerIndex(command.playerIndex);
                }
                break;
            }
            case Command::Type::setVibration:
            {
                break;
            }
            case Command::Type::showVirtualKeyboard:
                showVirtualKeyboard();
                break;
            case Command::Type::hideVirtualKeyboard:
                hideVirtualKeyboard();
                break;
            default:
                break;
        }
    }

    void InputSystem::startGamepadDiscovery()
    {
        [GCController startWirelessControllerDiscoveryWithCompletionHandler:
            ^(void) { handleGamepadDiscoveryCompleted(); }];
    }

    void InputSystem::stopGamepadDiscovery()
    {
        [GCController stopWirelessControllerDiscovery];
    }

    void InputSystem::handleGamepadDiscoveryCompleted()
    {
        sendEvent(Event(Event::Type::deviceDiscoveryComplete));
    }

    void InputSystem::handleGamepadConnected(GCControllerPtr controller)
    {
        std::vector<std::int32_t> playerIndices = {0, 1, 2, 3};

        for (const auto& i : gamepadDevices)
            if (const auto n = std::find(playerIndices.begin(), playerIndices.end(), i.second->getPlayerIndex()); n != playerIndices.end())
                playerIndices.erase(n);

        if (!playerIndices.empty())
            controller.playerIndex = static_cast<GCControllerPlayerIndex>(playerIndices.front());

        auto gamepadDevice = std::make_unique<GamepadDevice>(*this, getNextDeviceId(), controller);
        gamepadDevices.insert(std::make_pair(controller, std::move(gamepadDevice)));
    }

    void InputSystem::handleGamepadDisconnected(GCControllerPtr controller)
    {
        if (const auto i = gamepadDevices.find(controller); i != gamepadDevices.end())
            gamepadDevices.erase(i);
    }

    void InputSystem::showVirtualKeyboard()
    {
        const auto windowIOS = static_cast<core::ios::NativeWindow*>(engine->getWindow()->getNativeWindow());
        UITextField* textField = windowIOS->getTextField();
        [textField becomeFirstResponder];
    }

    void InputSystem::hideVirtualKeyboard()
    {
        const auto windowIOS = static_cast<core::ios::NativeWindow*>(engine->getWindow()->getNativeWindow());
        UITextField* textField = windowIOS->getTextField();
        [textField resignFirstResponder];
    }
}
