// Copyright 2015-2020 Elviss Strazdins. All rights reserved.

#ifndef OUZEL_GRAPHICS_METALRENDERDEVICEIOS_HPP
#define OUZEL_GRAPHICS_METALRENDERDEVICEIOS_HPP

#include "../../../core/Setup.h"

#if defined(__APPLE__)
#  include <TargetConditionals.h>
#endif

#if TARGET_OS_IOS && OUZEL_COMPILE_METAL

#include "../MetalRenderDevice.hpp"
#include "../../../core/ios/DisplayLink.hpp"

namespace ouzel::graphics::metal::ios
{
    class RenderDevice final: public metal::RenderDevice
    {
        friend Renderer;
    public:
        explicit RenderDevice(const std::function<void(const Event&)>& initCallback);
        ~RenderDevice() override;

        void renderCallback();

    private:
        void init(core::Window* newWindow,
                  const Size2U& newSize,
                  std::uint32_t newSampleCount,
                  bool newSrgb,
                  bool newVerticalSync,
                  bool newDepth,
                  bool newStencil,
                  bool newDebugRenderer) final;

        core::ios::DisplayLink displayLink;
    };
}

#endif

#endif // OUZEL_GRAPHICS_METALRENDERDEVICEIOS_HPP
