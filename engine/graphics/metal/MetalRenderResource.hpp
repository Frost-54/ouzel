// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_METALRENDERRESOURCE_HPP
#define OUZEL_GRAPHICS_METALRENDERRESOURCE_HPP

#include "../../core/Setup.h"

#if OUZEL_COMPILE_METAL

namespace ouzel::graphics::metal
{
    class RenderDevice;

    class RenderResource
    {
    public:
        explicit RenderResource(RenderDevice& initRenderDevice) noexcept:
            renderDevice(initRenderDevice)
        {
        }

        virtual ~RenderResource() = default;

        RenderResource(const RenderResource&) = delete;
        RenderResource& operator=(const RenderResource&) = delete;

        RenderResource(RenderResource&&) = delete;
        RenderResource& operator=(RenderResource&&) = delete;

    protected:
        RenderDevice& renderDevice;
    };
}

#endif

#endif // OUZEL_GRAPHICS_METALRENDERRESOURCE_HPP
