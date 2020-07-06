// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_MATH_COLOR_HPP
#define OUZEL_MATH_COLOR_HPP

#include <cstddef>
#include <cstdint>
#include <array>
#include <string>
#include <type_traits>
#include "Vector.hpp"

namespace ouzel
{
    class Color final
    {
    public:
        std::uint8_t v[4]{0};

        constexpr Color() noexcept {}

        explicit constexpr Color(std::uint32_t color) noexcept:
            v{
                static_cast<std::uint8_t>((color & 0xFF000000) >> 24),
                static_cast<std::uint8_t>((color & 0x00FF0000) >> 16),
                static_cast<std::uint8_t>((color & 0x0000FF00) >> 8),
                static_cast<std::uint8_t>(color & 0x000000FF)
            }
        {
        }

        explicit Color(const std::string& color)
        {
            if (!color.empty())
            {
                if (color.front() == '#')
                {
                    assert(color.length() == 4 || color.length() == 7);

                    const std::size_t componentSize = (color.length() - 1) / 3; // exclude the #

                    for (std::size_t component = 0; component < 3; ++component)
                    {
                        v[component] = 0;

                        for (std::size_t byte = 0; byte < 2; ++byte)
                        {
                            const char c = (byte < componentSize) ? color[component * componentSize + byte + 1] : color[component * componentSize + 1];
                            v[component] = static_cast<std::uint8_t>((v[component] << 4) | hexToInt(c));
                        }
                    }

                    v[3] = 0xFF; // alpha
                }
                else
                {
                    uint32_t intValue = 0;

                    for (const auto c : color)
                        intValue = intValue * 10 + decToInt(c);

                    v[0] = static_cast<std::uint8_t>((intValue & 0xFF000000) >> 24);
                    v[1] = static_cast<std::uint8_t>((intValue & 0x00FF0000) >> 16);
                    v[2] = static_cast<std::uint8_t>((intValue & 0x0000FF00) >> 8);
                    v[3] = static_cast<std::uint8_t>(intValue & 0x000000FF);
                }
            }
            else
                for (std::size_t i = 0; i < 4; ++i)
                    v[i] = 0;
        }

        explicit Color(const char* color):
            Color(std::string(color))
        {
        }

        template <class T, typename std::enable_if<std::is_integral<T>::value>::type* = nullptr>
        constexpr Color(T red, T green, T blue, T alpha = 0xFF) noexcept:
            v{std::uint8_t(red), std::uint8_t(green), std::uint8_t(blue), std::uint8_t(alpha)}
        {
        }

        constexpr Color(float red, float green, float blue, float alpha = 1.0F) noexcept:
            v{
                static_cast<std::uint8_t>(red * 255.0F),
                static_cast<std::uint8_t>(green * 255.0F),
                static_cast<std::uint8_t>(blue * 255.0F),
                static_cast<std::uint8_t>(alpha * 255.0F)
            }
        {
        }

        explicit Color(const float color[4]) noexcept:
            v{
                static_cast<std::uint8_t>(std::round(color[0] * 255.0F)),
                static_cast<std::uint8_t>(std::round(color[1] * 255.0F)),
                static_cast<std::uint8_t>(std::round(color[2] * 255.0F)),
                static_cast<std::uint8_t>(std::round(color[3] * 255.0F))
            }
        {
        }

        explicit Color(const Vector<3, float>& vec) noexcept:
            v{
                static_cast<std::uint8_t>(std::round(vec.v[0] * 255.0F)),
                static_cast<std::uint8_t>(std::round(vec.v[1] * 255.0F)),
                static_cast<std::uint8_t>(std::round(vec.v[2] * 255.0F)),
                0
            }
        {
        }

        explicit Color(const Vector<4, float>& vec) noexcept:
            v{
                static_cast<std::uint8_t>(std::round(vec.v[0] * 255.0F)),
                static_cast<std::uint8_t>(std::round(vec.v[1] * 255.0F)),
                static_cast<std::uint8_t>(std::round(vec.v[2] * 255.0F)),
                static_cast<std::uint8_t>(std::round(vec.v[3] * 255.0F))
            }
        {
        }

        static constexpr Color black() noexcept { return Color(0, 0, 0, 255); }
        static constexpr Color red() noexcept { return Color(255, 0, 0, 255); }
        static constexpr Color magenta() noexcept { return Color(255, 0, 255, 255); }
        static constexpr Color green() noexcept { return Color(0, 255, 0, 255); }
        static constexpr Color cyan() noexcept { return Color(0, 255, 255, 255); }
        static constexpr Color blue() noexcept { return Color(0, 0, 255, 255); }
        static constexpr Color yellow() noexcept { return Color(255, 255, 0, 255); }
        static constexpr Color white() noexcept { return Color(255, 255, 255, 255); }
        static constexpr Color gray() noexcept { return Color(128, 128, 128, 255); }

        std::uint8_t& operator[](std::size_t index) noexcept { return v[index]; }
        constexpr std::uint8_t operator[](std::size_t index) const noexcept { return v[index]; }

        std::uint8_t& r() noexcept { return v[0]; }
        constexpr std::uint8_t r() const noexcept { return v[0]; }

        std::uint8_t& g() noexcept { return v[1]; }
        constexpr std::uint8_t g() const noexcept { return v[1]; }

        std::uint8_t& b() noexcept { return v[2]; }
        constexpr std::uint8_t b() const noexcept { return v[2]; }

        std::uint8_t& a() noexcept { return v[3]; }
        constexpr std::uint8_t a() const noexcept { return v[3]; }

        constexpr std::array<float, 4> norm() const noexcept
        {
            return {{v[0] / 255.0F, v[1] / 255.0F, v[2] / 255.0F, v[3] / 255.0F}};
        }
        constexpr float normR() const noexcept { return v[0] / 255.0F; }
        constexpr float normG() const noexcept { return v[1] / 255.0F; }
        constexpr float normB() const noexcept { return v[2] / 255.0F; }
        constexpr float normA() const noexcept { return v[3] / 255.0F; }

        constexpr auto getIntValue() const noexcept
        {
            return (static_cast<std::uint32_t>(v[0]) << 24) |
                   (static_cast<std::uint32_t>(v[1]) << 16) |
                   (static_cast<std::uint32_t>(v[2]) << 8) |
                   static_cast<std::uint32_t>(v[3]);
        }

        constexpr bool operator<(const Color& c) const noexcept
        {
            return v[0] == c.v[0] ?
                v[1] == c.v[1] ?
                    v[2] == c.v[2] ?
                        v[3] == c.v[3] ?
                            false :
                            v[3] < c.v[3] :
                        v[2] < c.v[2] :
                    v[1] < c.v[1] :
                v[0] < c.v[0];
        }

        constexpr bool operator==(const Color& c) const noexcept
        {
            return v[0] == c.v[0] &&
                v[1] == c.v[1] &&
                v[2] == c.v[2] &&
                v[3] == c.v[3];
        }

        constexpr bool operator!=(const Color& c) const noexcept
        {
            return v[0] != c.v[0] ||
                v[1] != c.v[1] ||
                v[2] != c.v[2] ||
                v[3] != c.v[3];
        }

        constexpr auto isZero() const noexcept
        {
            return v[0] == 0 &&
                v[1] == 0 &&
                v[2] == 0 &&
                v[3] == 0;
        }

    private:
        static constexpr std::uint8_t hexToInt(const char c)
        {
            return (c >= '0' && c <= '9') ? static_cast<std::uint8_t>(c - '0') :
                (c >= 'a' && c <= 'f') ? static_cast<std::uint8_t>(c - 'a' + 10) :
                (c >= 'A' && c <= 'F') ? static_cast<std::uint8_t>(c - 'A' + 10) :
                throw std::out_of_range("Invalid hex digit");
        }

        static constexpr std::uint8_t decToInt(const char c)
        {
            return (c >= '0' && c <= '9') ? static_cast<std::uint8_t>(c - '0') :
                throw std::out_of_range("Invalid hex digit");
        }
    };
}

#endif // OUZEL_MATH_COLOR_HPP
