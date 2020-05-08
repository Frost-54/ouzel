// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_COLORMASK_HPP
#define OUZEL_GRAPHICS_COLORMASK_HPP

#include <type_traits>

namespace ouzel
{
    namespace graphics
    {
        enum class ColorMask: std::uint8_t
        {
            Red = 0x01,
            Green = 0x02,
            Blue = 0x04,
            Alpha = 0x08,
            All = Red | Green | Blue | Alpha
        };

        inline constexpr ColorMask operator&(const ColorMask a, const ColorMask b) noexcept
        {
            return static_cast<ColorMask>(static_cast<std::underlying_type_t<ColorMask>>(a) & static_cast<std::underlying_type_t<ColorMask>>(b));
        }
        inline constexpr ColorMask operator|(const ColorMask a, const ColorMask b) noexcept
        {
            return static_cast<ColorMask>(static_cast<std::underlying_type_t<ColorMask>>(a) | static_cast<std::underlying_type_t<ColorMask>>(b));
        }
        inline constexpr ColorMask operator^(const ColorMask a, const ColorMask b) noexcept
        {
            return static_cast<ColorMask>(static_cast<std::underlying_type_t<ColorMask>>(a) ^ static_cast<std::underlying_type_t<ColorMask>>(b));
        }
        inline constexpr ColorMask operator~(const ColorMask a) noexcept
        {
            return static_cast<ColorMask>(~static_cast<std::underlying_type_t<ColorMask>>(a));
        }
        inline constexpr ColorMask& operator&=(ColorMask& a, const ColorMask b) noexcept
        {
            return a = static_cast<ColorMask>(static_cast<std::underlying_type_t<ColorMask>>(a) & static_cast<std::underlying_type_t<ColorMask>>(b));
        }
        inline constexpr ColorMask& operator|=(ColorMask& a, const ColorMask b) noexcept
        {
            return a = static_cast<ColorMask>(static_cast<std::underlying_type_t<ColorMask>>(a) | static_cast<std::underlying_type_t<ColorMask>>(b));
        }
        inline constexpr ColorMask& operator^=(ColorMask& a, const ColorMask b) noexcept
        {
            return a = static_cast<ColorMask>(static_cast<std::underlying_type_t<ColorMask>>(a) ^ static_cast<std::underlying_type_t<ColorMask>>(b));
        }
    } // namespace graphics
} // namespace ouzel

#endif // OUZEL_GRAPHICS_COLORMASK_HPP
