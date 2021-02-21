// Ouzel by Elviss Strazdins

#ifndef OUZEL_INPUT_NATIVECURSORLINUX_HPP
#define OUZEL_INPUT_NATIVECURSORLINUX_HPP

#include <cstdint>
#include <vector>

#include "../../core/Setup.h"
#if OUZEL_SUPPORTS_X11
#  include <X11/X.h>
#endif

#include "../Cursor.hpp"
#include "../../math/Size.hpp"

namespace ouzel::input::linux
{
    class Cursor final
    {
    public:
        explicit Cursor(SystemCursor systemCursor);
        Cursor(const std::vector<std::uint8_t>& data,
                    const Size2F& size,
                    graphics::PixelFormat pixelFormat,
                    const Vector2F& hotSpot);
        ~Cursor();

        Cursor(const Cursor&) = delete;
        Cursor& operator=(const Cursor&) = delete;
        Cursor(Cursor&&) = delete;
        Cursor& operator=(Cursor&&) = delete;

#if OUZEL_SUPPORTS_X11
        auto getCursor() const noexcept { return cursor; }
#endif

    private:
#if OUZEL_SUPPORTS_X11
        ::Cursor cursor = None;
#endif
    };
}

#endif // OUZEL_INPUT_NATIVECURSORLINUX_HPP
