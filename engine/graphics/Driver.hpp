// Copyright 2015-2021 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_DRIVER_HPP
#define OUZEL_GRAPHICS_DRIVER_HPP

namespace ouzel::graphics
{
    enum class Driver
    {
        empty,
        openGL,
        direct3D11,
        metal
    };
}

#endif // OUZEL_GRAPHICS_DRIVER_HPP
