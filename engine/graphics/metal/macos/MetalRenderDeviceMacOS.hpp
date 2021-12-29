// Ouzel by Elviss Strazdins

#ifndef OUZEL_GRAPHICS_METALRENDERDEVICEMACOS_HPP
#define OUZEL_GRAPHICS_METALRENDERDEVICEMACOS_HPP

#include "../../../core/Setup.h"

#ifdef __APPLE__
#  include <TargetConditionals.h>
#endif

#if TARGET_OS_MAC && !TARGET_OS_IOS && !TARGET_OS_TV && OUZEL_COMPILE_METAL

#include "../MetalRenderDevice.hpp"
#include "../../../events/EventHandler.hpp"
#include "../../../platform/corevideo/DisplayLink.hpp"

namespace ouzel::graphics::metal::macos
{
    class RenderDevice final: public metal::RenderDevice
    {
        friend Graphics;
    public:
        RenderDevice(const Settings& settings,
                     core::Window& initWindow);
        ~RenderDevice() override;

        std::vector<math::Size<std::uint32_t, 2>> getSupportedResolutions() const final;
        void start() final;
        void renderCallback();

    private:
        void changeScreen(const std::uintptr_t) final;

        platform::corevideo::DisplayLink displayLink;

        std::atomic_bool running{false};
    };
}

#endif

#endif // OUZEL_GRAPHICS_METALRENDERDEVICEMACOS_HPP
