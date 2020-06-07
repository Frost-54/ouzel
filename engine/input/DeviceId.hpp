// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_INPUT_DEVICEID_HPP
#define OUZEL_INPUT_DEVICEID_HPP

#include <typeindex> 

namespace ouzel
{
    namespace input
    {
        struct DeviceId
        {
            std::uintptr_t value = 0;

            constexpr bool operator==(DeviceId other) const noexcept
            {
                return value == other.value;
            }
        };
    } // namespace input
} // namespace ouzel

namespace std
{
    template <> struct hash<ouzel::input::DeviceId>
    {
        std::size_t operator()(const ouzel::input::DeviceId& deviceId) const noexcept
        {
            return hash<std::uintptr_t>()(deviceId.value);
        }
    };
}

#endif // OUZEL_INPUT_DEVICEID_HPP
