// Copyright 2015-2021 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_BLENDFACTOR_HPP
#define OUZEL_GRAPHICS_BLENDFACTOR_HPP

namespace ouzel::graphics
{
    enum class BlendFactor
    {
        zero,
        one,
        srcColor,
        invSrcColor,
        srcAlpha,
        invSrcAlpha,
        destAlpha,
        invDestAlpha,
        destColor,
        invDestColor,
        srcAlphaSat,
        blendFactor,
        invBlendFactor
    };
}

#endif // OUZEL_GRAPHICS_BLENDFACTOR_HPP
